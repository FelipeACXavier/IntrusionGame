#include <fstream>
#include <iostream>
#include <memory>

#include <argumentum/argparse.h>

#include "game.h"
#include "statistics.h"

using namespace argumentum;
using json = nlohmann::json;

int main (int argc, char **argv)
{
  Args args;
  std::string configFile;

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

  if (!parser.parse_args(argc, argv, 1))
    return 1;

  std::ifstream f(configFile);
  auto configs = json::parse(f);

  bool running = true;
  uint32_t iterations = args.iterations > 0 ? args.iterations : uint32_t(configs["iterations"]);

  std::cout << "Running game with config: " << configFile << std::endl;
  std::cout << "Test type: " << configs["test_type"] << std::endl;
  std::cout << "Observed value: " << configs["observed_mean"]<< std::endl;
  std::cout << "Running " << args.batches << " batches and " <<  iterations << " iterations" << std::endl;

  Statistics stats(configs["test_type"], args.batches, iterations);

  for (uint32_t i = 0; running && i < args.batches; ++i)
  {
    std::unique_ptr<Game> game = std::make_unique<Game>(configs);
    if (!game->Init(args))
      return 1;

    stats.NewBatch();
    for (uint32_t j = 0; running && j < iterations; ++j)
    {
      running = game->Run();
      stats.UpdateStats(j, game->GetResult());
      game->Reset();
    }
    printf("Done with %u out of %u batches\n", i, args.batches);
    stats.Dump();
  }

  printf("Done running %u simulations\n", args.batches * iterations);

  stats.Save();
  observed = observed < 0.0 ? float(configs["observed_mean"]) : observed;
  stats.ZTest(configs["test_type"], confidence, observed);

  return 0;
}