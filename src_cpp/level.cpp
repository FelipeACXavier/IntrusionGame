#include "level.h"

#include <fstream>
#include <iostream>

#include "helpers.h"

using json = nlohmann::json;

Level::Level()
{
}

Level::~Level()
{
}

bool Level::Init(const nlohmann::json& config, SDL_Renderer* renderer)
{
  LOG_AND_RETURN_ON_FAILURE(CreateDoors(config["doors"], renderer), "Failed to create doors");
  LOG_AND_RETURN_ON_FAILURE(CreateAttacker(config["attacker"], renderer), "Failed to create attacker");
  LOG_AND_RETURN_ON_FAILURE(CreateEmployees(config["employees"], renderer), "Failed to create employees");
  LOG_AND_RETURN_ON_FAILURE(CreateGuards(config["guards"], renderer), "Failed to create guards");

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
      mGuards.push_back(std::make_shared<Guard>(i, config["config"][index], movables, renderer));
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
    mAttacker = std::make_shared<Attacker>(config, mDoors, renderer);
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
      mEmployees.push_back(std::make_shared<Employee>(i, config, renderer));
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return false;
  }

  return true;
}
