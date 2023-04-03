#include "statistics.h"

#include <algorithm>
#include <cmath>
#include <fstream>

#include <stdio.h>

#include "helpers.h"

Statistics::Statistics(const std::string& type, uint32_t batches, uint32_t iterations)
    : mBatches(batches)
    , mBatchIndex(-1)
    , mIterations(iterations)
{
  mType = type == "q-test" ? TestType::Q_TEST : TestType::P_TEST;
  mStart = std::chrono::system_clock::now();
  mPreviousEnd = mStart;
}

Statistics::~Statistics()
{
}

void Statistics::UpdateStats(uint32_t iteration, Result result)
{
  if (result.success)
    ++(mStats[mBatchIndex]->wins);
  else
    ++(mStats[mBatchIndex]->losses);

  mStats[mBatchIndex]->doorsEntered += float(result.doorStats.successes);
  mStats[mBatchIndex]->doorsBlocked += float(result.doorStats.failures);

  mStats[mBatchIndex]->pSamples.push_back(PValue(*mStats[mBatchIndex]));
  mStats[mBatchIndex]->qSamples.push_back(result.ticksElapsed / float(DAY_LENGTH * 60));

  IterationDone(iteration, *mStats[mBatchIndex]);
}

void Statistics::IterationDone(uint32_t iteration, const GameStats& stat)
{
  auto now = std::chrono::system_clock::now();
  if (mType == TestType::P_TEST)
    printf("Attempt %u has p=%.6f\n", iteration, PValue(stat));
  else
    printf("Attempt %u has q=%.6f\n", iteration, QValue(stat));

  // printf("Iteration done in %ld milliseconds\n", std::chrono::duration_cast<std::chrono::milliseconds>(now - mPreviousEnd).count());
  mPreviousEnd = now;
}

void Statistics::NewBatch()
{
  ++mBatchIndex;
  GameStats stat;
  mStats.push_back(std::make_shared<GameStats>());
}

void Statistics::Dump() const
{
  auto stat = mStats[mBatchIndex];
  printf("=========================================\n");
  printf("The attacker won %u games and lost %u\n", stat->wins, stat->losses);
  printf("Entered %.0f and blocked %.0f doors \n", stat->doorsEntered, stat->doorsBlocked);
  printf("Calculated p value = %.6f\n", PValue(*stat));
  printf("Calculated q value = %.6f\n", QValue(*stat));
  printf("=========================================\n");
}

float Statistics::PValue(const GameStats& stat) const
{
  if (stat.doorsEntered == 0 && stat.doorsBlocked == 0)
    return 0.0;

  return stat.doorsEntered / (stat.doorsEntered + stat.doorsBlocked);
}

float Statistics::QValue(const GameStats& stat) const
{
  return Mean(stat.qSamples);
}

bool Statistics::ZTest(const std::string& confidence, float observed) const
{
  if (mZTable.find(confidence) == mZTable.end())
  {
    printf("Unknown confidence level provided: %s\n", confidence.c_str());
    return false;
  }

  TestStats stats = GetStats();

  // Perform test
  float deviation = std::sqrt(stats.variance / stats.samples.size());
  float zValue = std::abs((observed - stats.mean) / deviation);
  float pValue = mZTable.at(confidence);

  printf("Z-test with:\n");
  printf("  %s\n", mBatches > 1 ? "Batch method" : "Long run method");
  printf("  Observed mean: %.6f\n", observed);
  printf("  Expected mean: %.6f\n", stats.mean);
  printf("  Expected variance: %.6f\n", stats.variance);
  printf("  Pass if: %.4f < %.4f\n", zValue, pValue);

  return true;
}

bool Statistics::Save(const std::string& filename) const
{
  auto now = std::chrono::system_clock::now();
  printf("Simulation done in %ld milliseconds\n", std::chrono::duration_cast<std::chrono::milliseconds>(now - mStart).count());

  std::ofstream file(filename, std::ios::trunc);
  LOG_AND_RETURN_ON_FAILURE(file.is_open(), std::string("Could not open request file: " + filename).c_str());

  // Get samples to be saved
  TestStats stats = GetStats();

  file << stats.mean << "\n";
  file << stats.variance << "\n";
  for (const auto& s : stats.samples)
    file << s << "\n";

  file.close();

  return true;
}

float Statistics::Mean(const std::vector<float>& samples) const
{
  if (samples.empty())
    return 0;

  float mean = 0.0;

  std::for_each(samples.begin(), samples.end(), [&mean](float n){ mean += n; });

  return mean / samples.size();
}

float Statistics::Variance(const std::vector<float>& samples) const
{
  if (samples.empty())
    return 0;

  float variance = 0.0;
  float mean = Mean(samples);

  std::for_each(samples.begin(), samples.end(), [&variance, mean](float n){ variance += std::pow(n - mean, 2); });

  return variance / samples.size();
}

Statistics::TestStats Statistics::GetStats() const
{
  TestStats stats;

  std::vector<float> qSamples;
  std::vector<float> pSamples;

  // If the batch method was used then we have to take the
  // mean across the batches instead of across the samples
  if (mBatches > 1)
  {
    for (auto& stat : mStats)
    {
      pSamples.push_back(PValue(*stat));
      qSamples.push_back(QValue(*stat));
    }
  }
  else
  {
    qSamples = mStats[mBatchIndex]->qSamples;
    pSamples = mStats[mBatchIndex]->pSamples;
  }

  if (mType == TestType::Q_TEST)
  {
    stats.mean = Mean(qSamples);
    stats.variance = Variance(qSamples);
    stats.samples = qSamples;
  }
  else
  {
    stats.mean = Mean(pSamples);
    stats.variance = Variance(pSamples);
    stats.samples = pSamples;
  }

  return stats;
}