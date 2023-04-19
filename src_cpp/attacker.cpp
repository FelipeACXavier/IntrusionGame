#include "attacker.h"

#include <string>

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
  mSpeed = float(config["speed"]) * TILE_SIZE;

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

  mAttackSpeed = config.contains("attack_speed") ? float(config["attack_speed"]) : mSpeed;

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

  int index = rand() % mDoors.size();
  mSelectedDoor = mDoors.at(index);
}

void Attacker::ResetPosition()
{
  Movable::Move(mSpeed);

  mStaying = true;
  mCanAttack = false;
  mSelectedDoor = nullptr;

  // Wait for next attempt in a few minutes
  mWaitTime = mAttackPeriod;
  mStayTime = mStayPeriod;
}

void Attacker::Move(float speed)
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

  if (mStayTime > 0)
  {
    --mStayTime;
    return;
  }

  SelectDoor();

  if (mSelectedDoor && mCanAttack)
  {
    float dX = (mSelectedDoor->X() - X());
    float dY = (mSelectedDoor->Y() - Y());

    if (std::abs(dX) > HALF_TILE || std::abs(dY) > HALF_TILE)
    {
      if (mBehaviour == Behaviour::WALK)
      {
        mDir.x = dX;
        mDir.y = dY;

        mPos.x += mDir.x * mAttackSpeed;
        mPos.y += mDir.y * mAttackSpeed;
      }
      else
      {
        mPos.x = mSelectedDoor->X();
        mPos.y = mSelectedDoor->Y();
      }
    }
    else
    {
      if (mSelectedDoor->Enter())
      {
        if (mStrategy == Strategy::P_TEST)
          ResetPosition();
        else if (mReachedDoor)
          mReachedDoor();
      }
      else
      {
        if (mStrategy == Strategy::P_TEST)
          ResetPosition();
      }

      mStaying = true;
      mCanAttack = false;
      mSelectedDoor = nullptr;
      mWaitTime = mAttackPeriod;
    }
  }
  else
  {
    Movable::Move(mSpeed);
  }

  if (mStaying)
    mStayTime = mStayPeriod;
}
