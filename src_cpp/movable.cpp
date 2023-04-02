#include "movable.h"

#include <iostream>

#include "helpers.h"

Movable::Movable(int x, int y, const std::vector<Line> walls, SDL_Renderer* renderer)
    : mRenderer(renderer)
    , mIsChecking(false)
    , mWalls(walls)
{

  mPos.x = x;
  mPos.y = y;

  mRandom = std::make_unique<Randomizer>(-1, 1);
  mRandomWidth = std::make_unique<Randomizer>(0, WIDTH);
  mRandomHeight = std::make_unique<Randomizer>(0, HEIGHT);
}

Movable::~Movable()
{
}

float Movable::X() const
{
  return mPos.x;
}

float Movable::Y() const
{
  return mPos.y;
}

void Movable::StartCheck()
{
  mIsChecking = true;
}

void Movable::StopCheck()
{
  mIsChecking = false;
}

bool Movable::IsChecking() const
{
  return mIsChecking;
}

void Movable::Constrain(float speed)
{
  // Ensure objects dont leave the scene
  if (floor(X()) + HALF_TILE > WIDTH)
    mPos.x = WIDTH - HALF_TILE;
  else if (floor(X()) - HALF_TILE < 0)
    mPos.x = HALF_TILE;

  if (floor(Y()) + HALF_TILE> HEIGHT)
    mPos.y = HEIGHT - HALF_TILE;
  else if (floor(Y()) - HALF_TILE < 0)
    mPos.y = HALF_TILE;
}

void Movable::Move(float speed)
{
  if (IsChecking())
    return;

  mDir.x = mRandom->Uniform();
  mDir.y = mRandom->Uniform();

  Point newPos(mPos.x + (mDir.x * speed), mPos.y + (mDir.y * speed));
  Point minPoint = Raycast(mPos, newPos, mWalls);

  if (minPoint == newPos)
  {
    mPos = newPos;
  }

  Constrain(speed);
}

void Movable::Update()
{
  Move(mSpeed);

  if (HIDDEN)
    return;

  SDL_SetRenderDrawColor(mRenderer, mColor.r, mColor.g, mColor.b, 255);
  // SDL_RenderDrawPoint(mRenderer, mPos.x, mPos.y);

  DrawCircle(mPos.x, mPos.y, HALF_TILE);
}

void Movable::SetColor(uint8_t r, uint8_t g, uint8_t b)
{
  mColor.r = r;
  mColor.g = g;
  mColor.b = b;
}

void Movable::DrawCircle(int32_t centreX, int32_t centreY, int32_t radius)
{
   const int32_t diameter = (radius * 2);

   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);

   while (x >= y)
   {
      //  Each of the following renders an octant of the circle
      SDL_RenderDrawPoint(mRenderer, centreX + x, centreY - y);
      SDL_RenderDrawPoint(mRenderer, centreX + x, centreY + y);
      SDL_RenderDrawPoint(mRenderer, centreX - x, centreY - y);
      SDL_RenderDrawPoint(mRenderer, centreX - x, centreY + y);
      SDL_RenderDrawPoint(mRenderer, centreX + y, centreY - x);
      SDL_RenderDrawPoint(mRenderer, centreX + y, centreY + x);
      SDL_RenderDrawPoint(mRenderer, centreX - y, centreY - x);
      SDL_RenderDrawPoint(mRenderer, centreX - y, centreY + x);

      if (error <= 0)
      {
         ++y;
         error += ty;
         ty += 2;
      }

      if (error > 0)
      {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
   }
}