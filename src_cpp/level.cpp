#include "level.h"

#include <fstream>
#include <iostream>

#include "helpers.h"

using json = nlohmann::json;

Level::Level(SDL_Renderer* renderer)
    : mRenderer(renderer)
{
}

Level::~Level()
{
}

bool Level::Init(const nlohmann::json& config)
{
  LOG_AND_RETURN_ON_FAILURE(CreateWalls(config["walls"]), "Failed to create walls");
  LOG_AND_RETURN_ON_FAILURE(CreateDoors(config["doors"], mRenderer), "Failed to create doors");
  LOG_AND_RETURN_ON_FAILURE(CreateAttacker(config["attacker"], mRenderer), "Failed to create attacker");
  LOG_AND_RETURN_ON_FAILURE(CreateEmployees(config["employees"], mRenderer), "Failed to create employees");
  LOG_AND_RETURN_ON_FAILURE(CreateGuards(config["guards"], mRenderer), "Failed to create guards");

  return true;
}

bool Level::Run()
{
  for (auto& employee : mEmployees)
    employee->Update();

  for (auto& door : mDoors)
    door->Update();

  for (auto& guard : mGuards)
    guard->Update();

  mAttacker->Update();

  UpdateWalls();

  return true;
}

DoorStats Level::GetResult()
{
  DoorStats stats;
  for (const auto& door : mDoors)
  {
    auto s = door->GetStats();
    stats.successes += s.successes;
    stats.failures += s.failures;
  }
  return stats;
}

void Level::UpdateWalls() const
{
  if (HIDDEN)
    return;

  SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
  for (auto& wall : mWalls)
    SDL_RenderDrawLine(mRenderer, wall.p1.x, wall.p1.y, wall.p2.x, wall.p2.y);
}

bool Level::CreateWalls(const nlohmann::json& config)
{
  try
  {
    for (auto& line : config)
    {
      mWalls.push_back(Line(line["x1"], line["y1"], line["x2"], line["y2"]));
      bool hasX = line.contains("dead_x");
      bool hasY = line.contains("dead_y");
      if (!hasX && !hasY)
        continue;

      if (hasX)
      {
        mWalls.back().deadzone.x = line["dead_x"] == "right" ? float(line["x1"]) : 0;
        mWalls.back().deadzone.w = line["dead_x"] == "right" ? WIDTH - float(line["x1"]) : float(line["x1"]);
      }
      else
      {
        mWalls.back().deadzone.x = float(line["x1"]);
        mWalls.back().deadzone.w = float(line["x2"]) - float(line["x1"]);
      }

      if (hasY)
      {
        mWalls.back().deadzone.y = line["dead_y"] == "bottom" ? float(line["y1"]) : 0;
        mWalls.back().deadzone.h = line["dead_y"] == "bottom" ? HEIGHT - float(line["y1"]) : float(line["y2"]);
      }
      else
      {
        mWalls.back().deadzone.y = float(line["y1"]);
        mWalls.back().deadzone.h = float(line["y2"]) - float(line["y1"]);
      }
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return false;
  }

  return true;
}

bool Level::CreateDoors(const nlohmann::json& config, SDL_Renderer* renderer)
{
  try
  {
    for (uint32_t i = 0; i < config.size(); i++)
      mDoors.push_back(std::make_shared<Door>(i, config[i], renderer));
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return false;
  }

  return true;
}

bool Level::CreateGuards(const nlohmann::json& config, SDL_Renderer* renderer)
{
  // Create list of movables so guard can iterate through attackers and employees as one
  std::vector<PMovable> movables(mEmployees.begin(), mEmployees.end());
  movables.push_back(mAttacker);

  try
  {
    uint32_t index = 0;
    auto nGuards = config["number_of_guards"];
    for (uint32_t i = 0; i < nGuards; ++i)
    {
      index = config["config"].size() == 1 ? 0 : index;
      mGuards.push_back(std::make_shared<Guard>(i, config["config"][index], movables, mWalls, renderer));
      index++;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return false;
  }


  return true;
}

bool Level::CreateAttacker(const nlohmann::json& config, SDL_Renderer* renderer)
{
  try
  {
    mAttacker = std::make_shared<Attacker>(config, mDoors, mWalls, renderer);
    mAttacker->mReachedDoor = mReachedDoor;
    mAttacker->mWasCaught = mWasCaught;
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return false;
  }

  return true;
}

bool Level::CreateEmployees(const nlohmann::json& config, SDL_Renderer* renderer)
{
  try
  {
    auto nEmployees = config["number_of_employees"];
    for (uint32_t i = 0; i < nEmployees; ++i)
      mEmployees.push_back(std::make_shared<Employee>(i, config, mWalls, renderer));
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return false;
  }

  return true;
}
