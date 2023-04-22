#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <nlohmann/json.hpp>

#include "door.h"
#include "guard.h"
#include "movable.h"
#include "randomizer.h"

class Attacker : public Movable
{
public:
  Attacker(const nlohmann::json& config, const std::vector<PDoor>& doors, const std::vector<Line> walls, SDL_Renderer* renderer);
  ~Attacker();

  void Move(const Point& goal) override;
  void StartCheck(int id) override;

  void SetGuards(const std::vector<PGuard>& guards);

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

  int mAttackSpeed;

  uint32_t mStayPeriod;
  uint32_t mStayTime;
  uint32_t mWaitTime;
  uint32_t mAttackPeriod;

  std::vector<PGuard> mGuards;

  void SelectDoor();
  void ResetPosition();
};

typedef std::shared_ptr<Attacker> PAttacker;