#pragma once

#include <stdint.h>
#include <memory>
#include <vector>

#include <SDL2/SDL.h>

#include "randomizer.h"
#include "settings.h"

class Movable
{
public:
  Movable(int x, int y, const std::vector<Line> walls, SDL_Renderer* renderer);
  ~Movable();

  virtual void Update();

  float X() const;
  float Y() const;

  virtual void StartCheck();
  virtual void StopCheck();

  bool IsChecking() const;

protected:
  Point mPos;
  Point mDir;
  int mSpeed;

  enum class State
  {
    IDLE = 0,
    ACTIVE
  } mState;

  Color mColor;

  SDL_Renderer* mRenderer;
  std::vector<Line> mWalls;

  std::unique_ptr<Randomizer> mRandom;
  std::unique_ptr<Randomizer> mRandomWidth;
  std::unique_ptr<Randomizer> mRandomHeight;

  std::vector<Point> mPoints;

  Point GetRandomPoint() const;

  virtual void Move(const Point& goal);
  virtual void Constrain(float speed);

  void SetColor(uint8_t r, uint8_t g, uint8_t b);
  void DrawCircle(int centreX, int centreY, int radius);
  void DrawPoints();

private:
  bool mIsChecking;
};

typedef std::shared_ptr<Movable> PMovable;