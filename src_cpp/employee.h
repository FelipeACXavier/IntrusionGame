#pragma once

#include <memory>

#include <nlohmann/json.hpp>

#include "movable.h"
#include "randomizer.h"

class Employee : public Movable
{
public:
  Employee(uint32_t id, const nlohmann::json& config, const std::vector<Line> walls, SDL_Renderer* renderer);
  ~Employee();

  void Move(float speed) override;
  // void Constrain(float speed) override;
  void StopCheck() override;

private:
  uint32_t mId;
  uint32_t mWaitTime;

  enum class Behaviour
  {
    STAY,
    WALK
  } mBehaviour;

  std::unique_ptr<Randomizer> mRandomWait;
};

typedef std::shared_ptr<Employee> PEmployee;