#include "employee.h"

Employee::Employee(uint32_t id, const nlohmann::json& config, SDL_Renderer* renderer)
    : Movable(0, 0, renderer)
    , mId(id)
    , mWaitTime(0)
{
  mPos.x = mRandomWidth->Uniform();
  mPos.y = mRandomHeight->Uniform();

  mSpeed = float(config["speed"]) * TILE_SIZE;

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

  mSpeed = float(config["speed"]) * TILE_SIZE;

  mRandomWait = std::make_unique<Randomizer>(minStayTime, maxStayTime);
}

Employee::~Employee()
{
}

void Employee::Move(float speed)
{
  if (mWaitTime)
  {
    --mWaitTime;
    return;
  }

  Movable::Move(speed);

  // Wait after every move
  mWaitTime = mRandomWait->Uniform();
}

void Employee::Constrain(float speed)
{
  if (X() > WIDTH)
    mPos.x = WIDTH - HALF_TILE;
  else if (X() < 0)
    mPos.x = HALF_TILE;

  if (Y() > HEIGHT)
    mPos.y = HEIGHT - HALF_TILE;
  else if (Y() < 0)
    mPos.y = HALF_TILE;
}

void Employee::StopCheck()
{
  Movable::StopCheck();

  // Move away from the guard check radius
  if (mBehaviour == Behaviour::WALK)
    Movable::Move(5 * mSpeed);

  // mWaitTime = 0
}
