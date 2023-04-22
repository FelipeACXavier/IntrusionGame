#include "movable.h"

#include <iostream>

#include "helpers.h"

Movable::Movable(int x, int y, const std::vector<Line> walls, SDL_Renderer* renderer)
    : mRenderer(renderer)
    , mWalls(walls)
    , mIsChecking(-1)
    , mState(State::IDLE)
    , mSpeed(0)
{

  mPos.x = x;
  mPos.y = y;

  mRandom = std::make_unique<Randomizer>(-1, 1);
  mRandomWidth = std::make_unique<Randomizer>(1, WIDTH - 1);
  mRandomHeight = std::make_unique<Randomizer>(1, HEIGHT - 1);

  mDir.x = mRandom->Uniform();
  mDir.y = mRandom->Uniform();
}

Movable::~Movable()
{
  mPoints.clear();
}

float Movable::X() const
{
  return mPos.x;
}

float Movable::Y() const
{
  return mPos.y;
}

Point Movable::Pos() const
{
  return mPos;
}

void Movable::StartCheck(int id)
{
  if (!IsChecking())
    mIsChecking = id;
}

void Movable::StopCheck(int id)
{
  if (id == mIsChecking)
    mIsChecking = -1;
}

bool Movable::IsChecking() const
{
  return mIsChecking != -1;
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

void Movable::Move(const Point& goal)
{
  if (IsChecking())
    return;

  if (mState == State::IDLE)
    return;

  if (mPoints.empty())
  {
    if (ToWorld(goal) != ToWorld(mPos))
      mPoints = PathFinding(mPos, goal, mWalls);
  }
  else
  {
    int index = 0;
    if (mPoints.size() > mSpeed)
      index = mSpeed;

    mPos = *(mPoints.begin() + index);

    if (!HIDDEN)
      DrawPoints();

    mPoints.erase(mPoints.begin(), mPoints.begin() + index + 1);

    // Only allow other behaviours once entity is in position
    if (mPoints.empty())
      mState = State::IDLE;
  }
}

void Movable::Update()
{
  Move(GetRandomPoint());

  if (HIDDEN)
    return;

  if (IsChecking())
    SDL_SetRenderDrawColor(mRenderer, 255, 127, 80, 255);
  else
    SDL_SetRenderDrawColor(mRenderer, mColor.r, mColor.g, mColor.b, 255);

  DrawCircle(mPos.x, mPos.y, 8);
}

Point Movable::GetRandomPoint() const
{
  bool ok;
  Point pos;

  do
  {
    ok = true;
    pos.x = mRandomWidth->Uniform();
    pos.y = mRandomHeight->Uniform();
    for (const auto& wall : mWalls)
    {
      SDL_Rect d = wall.deadzone;
      if (pos.x >= d.x && pos.x <= d.x + d.w &&
          pos.y >= d.y && pos.y <= d.y + d.h)
        ok = false;
    }
  } while (!ok);

  return pos;
}

void Movable::SetColor(uint8_t r, uint8_t g, uint8_t b)
{
  mColor.r = r;
  mColor.g = g;
  mColor.b = b;
}

void Movable::DrawPoints()
{
  SDL_SetRenderDrawColor(mRenderer, mColor.r, mColor.g, mColor.b, 255);

  for (int i = 1; i < mPoints.size(); ++i)
  {
    auto p1 = mPoints.at(i - 1);
    auto p2 = mPoints.at(i);
    DrawCircle(p1.x, p1.y, 3);
    SDL_RenderDrawLine(mRenderer, p1.x, p1.y, p2.x, p2.y);
  }
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