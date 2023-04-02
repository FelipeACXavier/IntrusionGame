#include "door.h"

#include <iostream>
#include <random>

Door::Door(uint32_t id, const nlohmann::json& config, SDL_Renderer* renderer)
    : mRenderer(renderer)
    , mId(id)
    , mIsOpen(false)
    , mIsNextLevel(true)
    , mWaitTime(0)
{
  srand (time(NULL));

  mPos.x = float(config["x"]) * WIDTH;
  mPos.y = float(config["y"]) * HEIGHT;

  CreateArea(config["direction"]);

  float maxOpenTime = float(config["max_open_time"]) * 60;
  float minOpenTime = float(config["min_open_time"]) * 60;
  float interDuration = float(config["inter_opening_time"]) * 60;
  float maxShortOpenTime = float(config["max_short_open_time"]) * 60;
  float minShortOpenTime = float(config["min_short_open_time"]) * 60;

  mShortOpeningProbability = 100 * float(config["short_opening_probability"]);

  mClosingRandom = std::make_unique<Randomizer>(interDuration, 1);
  mShortOpeningRandom = std::make_unique<Randomizer>(minShortOpenTime, maxShortOpenTime);
  mLongOpeningRandom = std::make_unique<Randomizer>(minOpenTime, maxOpenTime);
}

Door::~Door()
{
}

float Door::X() const
{
  return mPos.x;
}

float Door::Y() const
{
  return mPos.y;
}

DoorStats Door::GetStats() const
{
  return mStats;
}

void Door::CreateArea(const std::string& direction)
{
  mRect.x = mPos.x - HALF_TILE;
  mRect.y = mPos.y - HALF_TILE;
  mRect.w = TILE_SIZE;
  mRect.h = TILE_SIZE;

  if (mPos.x + HALF_TILE >= WIDTH)
    mRect.x -= TILE_SIZE / 2;
  else if (mPos.x - HALF_TILE <= 0)
    mRect.x += TILE_SIZE / 2;

  if (mPos.y + HALF_TILE >= HEIGHT)
    mRect.y -= TILE_SIZE / 2;
  else if (mPos.y - HALF_TILE <= 0)
    mRect.y += TILE_SIZE / 2;
}

bool Door::Enter()
{
  if (IsOpen())
  {
    ++mStats.successes;
    return true;
  }
  else
  {
    ++mStats.failures;
    return false;
  }
}

bool Door::IsOpen() const
{
  return mIsOpen;
}

bool Door::ToNextLevel() const
{
  return true;
}

void Door::React()
{
  // Check how long the door has been in current state
  if (mWaitTime > 0)
  {
    --mWaitTime;
    return;
  }

  if (IsOpen())
  {
    mWaitTime = mClosingRandom->Normal();
    mIsOpen = false;
  }
  else
  {
    // Check if the door should open for a short or long time
    if ((rand() % 100 + 1) <= mShortOpeningProbability)
      mWaitTime = mShortOpeningRandom->Uniform();
    else
      mWaitTime = mLongOpeningRandom->Uniform();

    mIsOpen = true;
  }
}

void Door::Update()
{
  React();

  if (HIDDEN)
    return;

  if (IsOpen())
    SDL_SetRenderDrawColor(mRenderer, 0, 255, 0, 255);
  else
    SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255);

  SDL_RenderDrawRect(mRenderer, &mRect);
}