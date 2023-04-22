#include "guard.h"

#include "stdio.h"
#include "helpers.h"

Guard::Guard(uint32_t id, const nlohmann::json& config,
             const std::vector<PMovable>& movables,
             const std::vector<Line>& walls,
             SDL_Renderer* renderer)
    : Movable(0, 0, walls, renderer)
    , mId(id)
    , mMovables(movables)
    , mCheckTime(0)
    , mMissionTime(0)
    , mInMission(false)
{
  // For rand() later
  srand (time(NULL));

  mPos = GetRandomPoint();

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

  Randomizer intermission(0, mInterMissionPeriod / 2);
  mWaitForMissionTime = intermission.Uniform();

  mMovablesPerCheck = uint32_t(config["entities_per_check"]);

  mInitialPos.x = mPos.x;
  mInitialPos.y = mPos.y;
}

Guard::~Guard()
{
}

float Guard::CheckRadius() const
{
  return mCheckRadius;
}

void Guard::Update()
{
  Movable::Update();

  if (HIDDEN)
    return;

  // Uncomment to see radius of detection
  // if (!mInMission)
  //   SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
  // DrawCircle(mPos.x, mPos.y, mShowRadius);
}

void Guard::Move(const Point& goal)
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
    StopCheck(mId);
    mBeingChecked.clear();
  }

  // See if we can perform check on every employee is found on the guards way
  if (mInMission)
    PerformCheck();

  mState = State::ACTIVE;
  Movable::Move(goal);

  // Wait a few minutes after position is reach
  // Maybe have coffee and look around
  if (!mInMission && mState == State::IDLE)
    mCheckTime = 120; // 4 minutes
}

void Guard::StartMission()
{
  if (mInMission)
    return;

  mCheckTime = 0;
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

  // Make sure all entities which were being checked are now free to roam again
  for (auto& m : mMovables)
    m->StopCheck(mId);

  mBeingChecked.clear();
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

    // Guards only check entities within a certain radius
    if (Distance(mPos.x, mPos.y, e->X(), e->Y()) >= mCheckRadius)
      continue;

    // Make sure we are not checking someone through a wall
    Point min = Raycast(mPos, e->Pos(), mWalls);
    if (min == e->Pos())
      possibleChecks.push_back(e);
  }

  if (possibleChecks.empty())
    return;

  // Of the possible checks, only a few are actually selected
  for (uint32_t i = 0; i < mMovablesPerCheck && !possibleChecks.empty(); ++i)
  {
    int index = rand() % possibleChecks.size();
    mBeingChecked.push_back(possibleChecks.at(index));
    possibleChecks.erase(possibleChecks.begin() + index);
  }

  StartCheck(mId);
  for (auto& e : mBeingChecked)
  {
    if (e->IsChecking())
      continue;

    e->StartCheck(mId);
  }

  // Get time to remain in place/checking
  mCheckTime = mRandomCheck->Uniform();
}

void Guard::ResetPosition()
{
  mPos.x = mInitialPos.x;
  mPos.y = mInitialPos.y;
}

void Guard::RayCast(std::vector<PMovable>& checks)
{
  for (auto iter = checks.begin(); iter < checks.end();)
  {
    Point p((*iter)->X(), (*iter)->Y());
    Point min = Raycast(mPos, p, mWalls);

    if (min == p)
      iter++;
    else
      iter = checks.erase(iter);
  }
}