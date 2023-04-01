#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "settings.h"

class Statistics
{
public:
  Statistics(const std::string& type, uint32_t batches, uint32_t iterations);
  ~Statistics();

  void UpdateStats(uint32_t iteration, Result result);

  void NewBatch();
  void Dump() const;

  void Save() const;

  bool ZTest(const std::string& type, const std::string& confidence, float observed) const;

  float Mean(const std::vector<float>& samples) const;
  float Variance(const std::vector<float>& samples) const;

private:
  uint32_t mBatches;
  uint32_t mBatchIndex;
  uint32_t mIterations;

  struct GameStats
  {
    uint32_t wins = 0;
    uint32_t losses = 0;
    float doorsEntered = 0.0;
    float doorsBlocked = 0.0;

    std::vector<float> pSamples;
    std::vector<float> qSamples;
  };

  std::vector<std::shared_ptr<GameStats>> mStats;

  enum class TestType
  {
    P_TEST,
    Q_TEST
  } mType;

  std::map<std::string, float> mZTable = {
    {"0.75", 1.15},
    {"0.90", 1.64},
    {"0.95", 1.96},
    {"0.99", 2.57}
  };

   std::chrono::_V2::system_clock::time_point mStart;
   std::chrono::_V2::system_clock::time_point mPreviousEnd;

  float PValue(const GameStats& stat) const;
  float QValue(const GameStats& stat) const;

  void IterationDone(uint32_t iteration, const GameStats& stat);
};