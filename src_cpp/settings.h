#pragma once
#include <stdint.h>

extern uint32_t FPS;
extern uint32_t CYCLES_PER_FRAME;
extern uint32_t DAY_LENGTH;

extern uint32_t WIDTH;
extern uint32_t HEIGHT;
extern uint32_t TILE_SIZE;
extern uint32_t HALF_TILE;

struct Point
{
  float x = 0;
  float y = 0;
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
  uint32_t ticksElapsed = 0;
  DoorStats doorStats;
};