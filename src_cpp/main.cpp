#include <fstream>
#include <memory>

#include <argumentum/argparse.h>

#include "game.h"
#include "helpers.h"
#include "statistics.h"

using namespace argumentum;
using json = nlohmann::json;

int main (int argc, char **argv)
{
  Args args;
  std::string configFile;
  std::string outDirectory;

  // Only used with the --stats option
  bool analysis;
  float observed;
  std::string confidence;

  auto parser = argument_parser{};
  auto params = parser.params();
  parser.config().program(argv[0]).description("Intrusion game simulator");
  params.add_parameter(configFile, "-c", "--config")
    .nargs(1)
    .help("Config file to be used");
  params.add_parameter(outDirectory, "--out-dir")
    .nargs(1)
    .absent("../data/")
    .help("Output directory");
  params.add_parameter(args.iterations, "-i")
    .nargs(1)
    .absent(0)
    .help("Override config iterations");
  params.add_parameter(args.batches, "-b")
    .nargs(1)
    .absent(1)
    .help("Override number of batches in config");
  params.add_parameter(args.cycles, "--cycles")
    .nargs(1)
    .absent(0)
    .help("Override number of cycles per frame in config");
  params.add_parameter(args.fps, "-f", "--fps")
    .nargs(1)
    .help("Override config fps");
  params.add_parameter(analysis, "--stats")
    .nargs(1)
    .absent(false)
    .help("Run only analysis from given file");
  params.add_parameter(observed, "-o")
    .nargs(1)
    .absent(-1.0)
    .help("Expected Z-test mean");
  params.add_parameter(confidence, "--confidence")
    .nargs(1)
    .absent("0.75")
    .help("Confidence to use in Z-test");
  params.add_parameter(args.hidden, "--hidden")
    .absent(false)
    .help("Do not show display when simulating");
  params.add_parameter(args.parameter, "--chg-param")
    .nargs(1)
    .absent("")
    .help("Overwrite this parameter in chg-entity");
  params.add_parameter(args.entity, "--chg-entity")
    .nargs(1)
    .absent("")
    .help("Overwrite chg-param in entity");
  params.add_parameter(args.value, "--chg-value")
    .nargs(1)
    .absent(FLT_MAX)
    .help("Overwrite chg-param in chg-entity to this value");

  if (!parser.parse_args(argc, argv, 1))
  {
    printf("Use %s -h or --help to view the options\n", argv[0]);
    return 1;
  }

  if (configFile.empty())
  {
    printf("No configuration file provided\n");
    return 1;
  }

  std::ifstream f(configFile);
  if (!f.is_open())
  {
    printf("Could not open file: %s\n", configFile.c_str());
    return 1;
  }

  auto configs = json::parse(f);
  bool running = true;
  uint32_t iterations = args.iterations > 0 ? args.iterations : uint32_t(configs["iterations"]);

  if (!args.entity.empty() && !args.parameter.empty())
  {
    printf("Overwriting %s in %s to %.2f\n", args.parameter.c_str(), args.entity.c_str(), args.value);
    try
    {
      if (args.entity == "guards")
      {
        if (args.parameter == "number_of_guards")
        {
          configs[args.entity][args.parameter] = args.value;
        }
        else
        {
          for (auto& c : configs[args.entity]["config"])
            c[args.parameter] = args.value;
        }
      }
      else
      {
        configs[args.entity][args.parameter] = args.value;
      }
    }
    catch (const std::exception& e)
    {
      printf("Error: %s\n", e.what());
      return -1;
    }
  }

  printf("Running game with config: %s\n", configFile.c_str());
  printf("Test type: %s\n", std::string(configs["test_type"]).c_str());
  printf("Observed value: %.6f\n", float(configs["observed_mean"]));
  printf("Running %u batches and %u iterations\n", args.batches, iterations);

  Statistics stats(configs["test_type"], args.batches, iterations);

  for (uint32_t i = 0; running && i < args.batches; ++i)
  {
    std::unique_ptr<Game> game = std::make_unique<Game>(configs);
    if (!game->Init(args))
      break;

    stats.NewBatch();
    for (uint32_t j = 0; running && j < iterations; ++j)
    {
      running = game->Run();
      stats.UpdateStats(j, game->GetResult());
      game->Reset();
    }
    printf("Done with %u out of %u batches\n", i + 1, args.batches);
    stats.Dump();
  }

  printf("Done running %u simulations\n", args.batches * iterations);

  std::string fileWithoutExtension = GetFilename(configFile) + "_" + (args.value == FLT_MAX ? GetDate() : std::to_string(int(args.value)));
  stats.Save(outDirectory + fileWithoutExtension + ".txt");

  observed = observed < 0.0 ? float(configs["observed_mean"]) : observed;
  stats.ZTest(confidence, observed);

  return 0;
}