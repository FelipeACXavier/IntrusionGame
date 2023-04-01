#pragma once

#include <functional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <SDL2/SDL.h>

#include "attacker.h"
#include "door.h"
#include "employee.h"
#include "guard.h"

class Level
{
public:
  Level();
  ~Level();

  bool Init(const nlohmann::json& config, SDL_Renderer* renderer);
  bool Run();

  DoorStats GetResult();

  std::function<void()> mReachedDoor;
  std::function<void()> mWasCaught;

private:
  PAttacker mAttacker;

  std::vector<PDoor> mDoors;
  std::vector<PGuard> mGuards;
  std::vector<PEmployee> mEmployees;

  bool CreateDoors(const nlohmann::json& config, SDL_Renderer* renderer);
  bool CreateGuards(const nlohmann::json& config, SDL_Renderer* renderer);
  bool CreateAttacker(const nlohmann::json& config, SDL_Renderer* renderer);
  bool CreateEmployees(const nlohmann::json& config, SDL_Renderer* renderer);
};