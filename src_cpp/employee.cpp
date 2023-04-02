#include "employee.h"

Employee::Employee(uint32_t id, const nlohmann::json& config, const std::vector<Line> walls, SDL_Renderer* renderer)
    : Movable(0, 0, walls, renderer)
    , mId(id)
    , mWaitTime(0)
{
  bool ok;
  do
  {
    ok = true;
    mPos.x = mRandomWidth->Uniform();
    mPos.y = mRandomHeight->Uniform();
    for (const auto& wall : walls)
    {
      if (mPos.x > wall.deadzone.x && mPos.x < wall.deadzone.x + wall.deadzone.w &&
          mPos.y > wall.deadzone.y && mPos.y < wall.deadzone.y + wall.deadzone.h)
        ok = false;
    }
  } while (!ok);

  // mPos.x = mRandomWidth->Uniform();
  // mPos.y = mRandomHeight->Uniform();

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

void Employee::StopCheck()
{
  Movable::StopCheck();

  // Move away from the guard check radius
  if (mBehaviour == Behaviour::WALK)
    Movable::Move(5 * mSpeed);

  // mWaitTime = 0
}
