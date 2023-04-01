#pragma once

#include <memory>
#include <string>
#include <random>

class Randomizer
{
public:
  Randomizer(float a, float b)
      : mGen(mRd())
      , mDist(a, b)
      , mNormalDist(a, b)
  {
  }

  ~Randomizer()
  {
  }

  float Uniform()
  {
    return mDist(mGen);
  }

  float Normal()
  {
    return mNormalDist(mGen);
  }

private:
  std::random_device mRd;
  std::mt19937 mGen;
  std::normal_distribution<float> mNormalDist;
  std::uniform_real_distribution<float> mDist;
};