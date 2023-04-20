#include "attacker.h"

#include <string>

#include "helpers.h"

Attacker::Attacker(const nlohmann::json& config, const std::vector<PDoor>& doors, const std::vector<Line> walls, SDL_Renderer* renderer)
  : Movable(0, 0, walls, renderer)
  , mDoors(doors)
  , mStaying(true)
  , mCanAttack(false)
{
  // For rand() later
  srand (time(NULL));

  int index = rand() % config["pos"].size();
  mPos.x = config["pos"][index]["x"];
  mPos.y = config["pos"][index]["y"];
  mSpeed = int(config["speed"]) - 1;

  SetColor(0, 0, 255);

  auto behaviour = config["behaviour"];
  auto strategy = config["strategy"];

  if (behaviour == "jump")
    mBehaviour = Behaviour::JUMP;
  else if (behaviour == "walk")
    mBehaviour = Behaviour::WALK;
  else
    throw std::runtime_error("Attacker behaviour is invalid");

  if (strategy == "p-test")
    mStrategy = Strategy::P_TEST;
  else if (strategy == "q-test")
    mStrategy = Strategy::Q_TEST;
  else if (strategy == "normal")
    mStrategy = Strategy::NORMAL;
  else
    throw std::runtime_error("Attacker strategy is invalid");

  mAttackSpeed = config.contains("attack_speed") ? int(config["attack_speed"]) - 1 : mSpeed;

  mStayPeriod = float(config["stay_period"]) * 60;
  mAttackPeriod = float(config["attack_period"]) * 60;
  mStayTime = mStayPeriod;
  mWaitTime = mAttackPeriod;
}

Attacker::~Attacker()
{
}

void Attacker::StartCheck()
{
  if (mWasCaught)
    mWasCaught();
}

void Attacker::SelectDoor()
{
  if (mSelectedDoor)
    return;

  if (mDoors.empty())
    return;

  mPoints.clear();

  int index = rand() % mDoors.size();
  mSelectedDoor = mDoors.at(index);
}

void Attacker::ResetPosition()
{
  Movable::Move(GetRandomPoint());

  mStaying = true;
  mCanAttack = false;
  mSelectedDoor = nullptr;

  // Wait for next attempt in a few minutes
  mWaitTime = mAttackPeriod;
  mStayTime = mStayPeriod;
}

void Attacker::Move(const Point& goal)
{
  // Count time until we try to attack
  if (mWaitTime > 0)
  {
    --mWaitTime;
  }
  else if (mStrategy != Strategy::Q_TEST)
  {
    mCanAttack = true;
    mStaying = false;
  }

  if (mStaying && mStayTime > 0)
  {
    --mStayTime;
    return;
  }

  SelectDoor();

  mState = State::ACTIVE;

  if (mSelectedDoor && mCanAttack)
  {
    if (ToWorld(mPos) != ToWorld(mSelectedDoor->Pos()))
    {
      Movable::Move(mSelectedDoor->Pos());
    }
    else
    {
      if (mSelectedDoor->Enter())
      {
        if (mStrategy != Strategy::P_TEST)
          mReachedDoor();
      }

      mStaying = true;
      mCanAttack = false;
      mSelectedDoor = nullptr;
      mWaitTime = mAttackPeriod;

      mState = State::IDLE;
    }
  }
  else
  {
    Movable::Move(goal);
  }

  if (mStaying && mState == State::IDLE)
    mStayTime = mStayPeriod;
}
