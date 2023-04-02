#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <nlohmann/json.hpp>

#include "door.h"
#include "movable.h"
#include "randomizer.h"

class Attacker : public Movable
{
public:
  Attacker(const nlohmann::json& config, const std::vector<PDoor>& doors, const std::vector<Line> walls, SDL_Renderer* renderer);
  ~Attacker();

  void Move(float speed) override;
  void StartCheck() override;

  std::function<void()> mReachedDoor;
  std::function<void()> mWasCaught;

private:
  PDoor mSelectedDoor;
  std::vector<PDoor> mDoors;

  bool mStaying;
  bool mCanAttack;

  enum class Strategy
  {
    P_TEST,
    Q_TEST,
    NORMAL
  } mStrategy;

  enum class Behaviour
  {
    WALK,
    JUMP
  } mBehaviour;

  float mAttackSpeed;

  uint32_t mStayPeriod;
  uint32_t mStayTime;
  uint32_t mWaitTime;
  uint32_t mAttackPeriod;

  void SelectDoor();
  void ResetPosition();
};

typedef std::shared_ptr<Attacker> PAttacker;