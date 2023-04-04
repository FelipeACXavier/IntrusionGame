#pragma once

#include <sys/stat.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include <cmath>
#include <string>
#include <vector>

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
      printf("%s\n", m);                 \
      return false;                      \
    }                                    \
  } while (0)

static bool CreateDirectory(const std::string& path)
{
  int r = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  return (r == 0 || errno == EEXIST);
}

static bool DoesFileExist(const std::string& file)
{
  return (access(file.c_str(), F_OK) != -1);
}

static std::string RemoveFilename(const std::string& path)
{
  size_t pos = path.find_last_of('/');
  if (pos == path.npos)
    return std::string("./");
  else
    return path.substr(0, pos + 1);
}
static std::string GetFilename(const std::string& path)
{
  size_t pos = path.find_last_of('/');
  auto fileWithExtension = pos == path.npos ? path : path.substr(pos + 1, path.npos);

  pos = fileWithExtension.find_last_of('.');
  return pos == path.npos ? fileWithExtension : fileWithExtension.substr(0, pos);
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