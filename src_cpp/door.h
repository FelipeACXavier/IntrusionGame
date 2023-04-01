#pragma once

#include <memory>

#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>

#include "randomizer.h"
#include "settings.h"

class Door
{
public:
  Door(uint32_t id, const nlohmann::json& config, SDL_Renderer* renderer);
  ~Door();

  float X() const;
  float Y() const;

  void Update();

  DoorStats GetStats() const;

  bool Enter();
  bool IsOpen() const;
  bool ToNextLevel() const;

private:
  SDL_Renderer* mRenderer;

  Point mPos;
  SDL_Rect mRect;

  const uint32_t mId;
  const bool mIsNextLevel;
  bool mIsOpen;

  DoorStats mStats;

  uint32_t mWaitTime;
  uint32_t mShortOpeningProbability;

  std::unique_ptr<Randomizer> mClosingRandom;
  std::unique_ptr<Randomizer> mShortOpeningRandom;
  std::unique_ptr<Randomizer> mLongOpeningRandom;

  void React();
  void CreateArea(const std::string& direction);
};

typedef std::shared_ptr<Door> PDoor;