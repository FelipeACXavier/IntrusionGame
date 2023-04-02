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
  float mSpeed;

  Color mColor;

  SDL_Renderer* mRenderer;
  std::vector<Line> mWalls;

  std::unique_ptr<Randomizer> mRandom;
  std::unique_ptr<Randomizer> mRandomWidth;
  std::unique_ptr<Randomizer> mRandomHeight;

  virtual void Move(float speed);
  virtual void Constrain(float speed);

  void SetColor(uint8_t r, uint8_t g, uint8_t b);
  void DrawCircle(int centreX, int centreY, int radius);

private:
  bool mIsChecking;
  // std::unique_ptr<Randomizer> mRandom;
};

typedef std::shared_ptr<Movable> PMovable;