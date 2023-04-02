#pragma once

#include <cmath>
#include <vector>

#include <SDL2/SDL.h>

#define RETURN_ON_FAILURE(c)             \
  do                                     \
  {                                      \
    if (!c)                              \
      return false;                      \
  } while (0)

#define LOG_AND_RETURN_ON_FAILURE(c,  m) \
  do                                     \
  {                                      \
    if (!c)                              \
    {                                    \
      printf(m);                         \
      return false;                      \
    }                                    \
  } while (0)

static bool checkCollision(float a, float b, float c, float& x, float& y, int radius)
{
  // Finding the distance of line from center.
  float dist = abs(a * x + b * y + c) / sqrt(a * a + b * b);

  // Checking if the distance is less than,
  // greater than or equal to radius.
  x = (b*(b*x - a*y) - a*c) / (a*a + b*b);
  y = (a*(-b*x + a*y) - b*c) / (a*a + b*b);

  return (radius >= dist);
}

static float Distance(float x1, float y1, float x2, float y2)
{
  return std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2);
}

static Point Raycast(const Point& p3, const Point& p4, const std::vector<Line>& lines)
{
  float x3 = p3.x;
  float y3 = p3.y;
  float x4 = p4.x;
  float y4 = p4.y;

  Point minPoint = p4;
  float minDistance = Distance(x3, y3, x4, y4);

  for (const auto& line : lines)
  {
    float x1 = line.p1.x;
    float y1 = line.p1.y;
    float x2 = line.p2.x;
    float y2 = line.p2.y;

    float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    // Ensure we dont divide by 0
    if (-0.01 < den && den < 0.01)
      continue;

    float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / den;
    float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / den;

    if (t > 0 && t < 1 && u > 0) {
      float x = x1 + t * (x2 - x1);
      float y = y1 + t * (y2 - y1);

      float dis = Distance(x3, y3, x, y);
      if (dis < (minDistance + 1000))
      {
        minDistance = dis;
        minPoint = Point(x, y);
      }
    }
  }

  return minPoint;
}