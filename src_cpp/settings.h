#pragma once
#include <stdint.h>
#include <SDL2/SDL.h>

extern uint32_t FPS;
extern uint32_t CYCLES_PER_FRAME;
extern uint32_t DAY_LENGTH;

extern uint32_t WIDTH;
extern uint32_t HEIGHT;
extern uint32_t TILE_SIZE;
extern uint32_t HALF_TILE;

extern bool HIDDEN;

struct Point
{
  Point(){}

  Point(float x_c, float y_c)
    : x(x_c)
    , y(y_c)
  {}

  bool operator==(Point const& p)
  {
    return x == p.x && y == p.y;
  }

  float x = 0;
  float y = 0;
};

struct Line
{
  Line() {}

  Line(float x1, float y1, float x2, float y2)
    : p1(x1, y1)
    , p2(x2, y2)
  {
    a = y1 - y2;
    b = x2 - x1;
    c = y1 * (x1 - x2) + x1 * (y2 - y1);
  }

  Point p1;
  Point p2;

  float a = 0.0;
  float b = 0.0;
  float c = 0.0;

  SDL_Rect deadzone;
};

struct DoorStats
{
  uint32_t successes = 0;
  uint32_t failures = 0;
};

struct Color
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct Args
{
  uint32_t fps = 0;
  uint32_t cycles = 0;
  uint32_t batches = 1;
  uint32_t iterations = 1;

  bool hidden = false;
};

struct Result
{
  bool success = false;
  float ticksElapsed = 0;
  DoorStats doorStats;
};