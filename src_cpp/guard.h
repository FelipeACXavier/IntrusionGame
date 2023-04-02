#pragma once

#include <memory>
#include <vector>

#include <nlohmann/json.hpp>

#include "movable.h"
#include "randomizer.h"
#include "settings.h"

class Guard : public Movable
{
public:
  Guard(uint32_t id, const nlohmann::json& config,
        const std::vector<PMovable>& movables,
        const std::vector<Line>& lines,
        SDL_Renderer* renderer);
  ~Guard();

  void Update() override;

private:
  uint32_t mId;
  float mCheckSpeed;
  float mStrollSpeed;

  std::vector<PMovable> mMovables;
  std::vector<PMovable> mBeingChecked;

  std::unique_ptr<Randomizer> mRandomCheck;
  std::unique_ptr<Randomizer> mRandomMission;

  bool mInMission;
  uint32_t mMissionTime;
  uint32_t mWaitForMissionTime;
  uint32_t mInterMissionPeriod;

  float mCheckRadius;
  float mShowRadius;
  uint32_t mCheckTime;
  uint32_t mMovablesPerCheck;

  enum class Behaviour
  {
    STROLL,
    RESET
  } mBehaviour;

  void Move(float speed) override;

  void StartMission();
  void StopMission();

  void PerformCheck();
  void ResetPosition();

  void RayCast(std::vector<PMovable>& checks);
};

typedef std::shared_ptr<Guard> PGuard;