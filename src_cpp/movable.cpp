#include "movable.h"

#include <iostream>

Movable::Movable(int x, int y, SDL_Renderer* renderer)
    : mRenderer(renderer)
    , mIsChecking(false)
{

  mPos.x = x;
  mPos.y = y;

  // std::cout << "Starting movable at " << mPos.x << ", " << mPos.y << std::endl;
  // mRandom = std::make_unique<Randomizer>(-1, 1);
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
  // if (X() > WIDTH || X() < 0)
  //   mPos.x += -(mDir.x * speed);

  // if (Y() > HEIGHT || Y() < 0)
  //   mPos.y += -(mDir.y * speed);

  if (X() > WIDTH)
    mPos.x = WIDTH - HALF_TILE;
  else if (X() < 0)
    mPos.x = HALF_TILE;

  if (Y() > HEIGHT)
    mPos.y = HEIGHT - HALF_TILE;
  else if (Y() < 0)
    mPos.y = HALF_TILE;
}

void Movable::Move(float speed)
{
  if (IsChecking())
    return;

  mPos.x = mRandomWidth->Uniform();
  mPos.y = mRandomHeight->Uniform();

  // mPos.x += (mDir.x * speed);
  // mPos.y += (mDir.y * speed);

  // std::cout << "Moving to " << mPos.x << ", " << mPos.y << std::endl;
}

void Movable::Update()
{
  Move(mSpeed);
  // Constrain(mSpeed);

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