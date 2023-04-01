#include "guard.h"

Guard::Guard(uint32_t id, const nlohmann::json& config, const std::vector<PMovable>& movables, SDL_Renderer* renderer)
    : Movable(0, 0, renderer)
    , mId(id)
    , mMovables(movables)
    , mCheckTime(0)
    , mMissionTime(0)
    , mInMission(false)
{
  // For rand() later
  srand (time(NULL));

  mPos.x = mRandomWidth->Uniform();
  mPos.y = mRandomHeight->Uniform();

  SetColor(255, 0, 0);

  mCheckSpeed = float(config["check_speed"]) * TILE_SIZE;
  mStrollSpeed = float(config["stroll_speed"]) * TILE_SIZE;
  mSpeed = mStrollSpeed;

  auto behaviour = config["behaviour"];
  if (behaviour == "stroll")
    mBehaviour = Behaviour::STROLL;
  else if (behaviour == "reset")
    mBehaviour = Behaviour::RESET;
  else
    throw std::runtime_error("Guard behaviour is invalid");

  mShowRadius = float(config["check_radius"]) * TILE_SIZE;
  mCheckRadius = std::pow(mShowRadius, 2.0);

  float maxCheckTime = float(config["max_check_time"]) * 60;
  float minCheckTime = float(config["min_check_time"]) * 60;
  mRandomCheck = std::make_unique<Randomizer>(minCheckTime, maxCheckTime);

  uint32_t dayInTicks = DAY_LENGTH * 60 * 60;
  mInterMissionPeriod = dayInTicks / float(config["number_of_missions"]);

  float maxMissionTime = float(config["max_mission_time"]) * 60;
  float minMissionTime = float(config["min_mission_time"]) * 60;
  mRandomMission = std::make_unique<Randomizer>(minMissionTime, maxMissionTime);

  Randomizer intermission(0, mInterMissionPeriod);
  mWaitForMissionTime = intermission.Uniform();

  mMovablesPerCheck = uint32_t(config["entities_per_check"]);
}

Guard::~Guard()
{
}

void Guard::Update()
{
  Movable::Update();

  // if (!mInMission)
  //   SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);

  // DrawCircle(mPos.x, mPos.y, mShowRadius);
}

void Guard::Move(float speed)
{
  if (mWaitForMissionTime > 0)
  {
    --mWaitForMissionTime;
    if (mBehaviour == Behaviour::RESET)
      return;
  }
  else
  {
    StartMission();
  }

  if (mMissionTime > 0)
    --mMissionTime;
  else
    StopMission();

  if (mCheckTime > 0)
  {
    --mCheckTime;
    return;
  }

  if (IsChecking())
  {
    StopCheck();
    for (auto& m : mBeingChecked)
      m->StopCheck();

    mBeingChecked.clear();
  }

  Movable::Move(mSpeed);

  if (mInMission)
    PerformCheck();
}

void Guard::StartMission()
{
  if (mInMission)
    return;

  mInMission = true;
  mSpeed = mCheckSpeed;
  mMissionTime = mRandomMission->Uniform();
}

void Guard::StopMission()
{
if (!mInMission)
    return;

  mInMission = false;
  mSpeed = mStrollSpeed;
  mWaitForMissionTime = mInterMissionPeriod;

  if (mBehaviour == Behaviour::RESET)
    ResetPosition();
}

void Guard::PerformCheck()
{
  if (!mBeingChecked.empty())
    return;

  std::vector<PMovable> possibleChecks;
  for (auto& e : mMovables)
  {
    if (e->IsChecking())
      continue;

    float dist = std::pow(X() - e->X(), 2) + std::pow(Y() - e->Y(), 2);
    if (dist <= mCheckRadius)
      possibleChecks.push_back(e);
  }

  if (possibleChecks.empty())
    return;

  for (uint32_t i = 0; i < mMovablesPerCheck; ++i)
  {
    int index = rand() % possibleChecks.size();
    mBeingChecked.push_back(possibleChecks.at(index));
    possibleChecks.erase(possibleChecks.begin() + index);
  }

  StartCheck();
  for (auto& e : mBeingChecked)
    e->StartCheck();

  // Get time to remain in place/checking
  mCheckTime = mRandomCheck->Uniform();
}

void Guard::ResetPosition()
{

}
