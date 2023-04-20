#include "employee.h"

Employee::Employee(uint32_t id, const nlohmann::json& config, const std::vector<Line> walls, SDL_Renderer* renderer)
    : Movable(0, 0, walls, renderer)
    , mId(id)
    , mWaitTime(0)
{
  mPos = GetRandomPoint();

  SetColor(0, 120, 50);

  auto behaviour = config["behaviour"];
  if (behaviour == "stay")
    mBehaviour = Behaviour::STAY;
  else if (behaviour == "walk")
    mBehaviour = Behaviour::WALK;
  else
    throw std::runtime_error("Employee behaviour is invalid");

  float maxStayTime = float(config["max_stay_time"]) * 60;
  float minStayTime = float(config["min_stay_time"]) * 60;

  mRandomWait = std::make_unique<Randomizer>(minStayTime, maxStayTime);
}

Employee::~Employee()
{
}

void Employee::Move(const Point& goal)
{
  if (mWaitTime)
  {
    --mWaitTime;
    return;
  }

  mState = State::ACTIVE;

  Movable::Move(goal);

  // Wait after every move
  if (mState == State::IDLE)
    mWaitTime = mRandomWait->Uniform();
}

void Employee::StopCheck()
{
  Movable::StopCheck();

  // Move away from the guard check radius
  if (mBehaviour == Behaviour::WALK)
    Movable::Move(GetRandomPoint());

  // mWaitTime = 0
}
