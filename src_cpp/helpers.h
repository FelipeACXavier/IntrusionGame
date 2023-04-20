#pragma once

#include <sys/stat.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>
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

static bool CreateDirectory(const std::string& openList)
{
  int r = mkdir(openList.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  return (r == 0 || errno == EEXIST);
}

static bool DoesFileExist(const std::string& file)
{
  return (access(file.c_str(), F_OK) != -1);
}

static std::string RemoveFilename(const std::string& openList)
{
  size_t pos = openList.find_last_of('/');
  if (pos == openList.npos)
    return std::string("./");
  else
    return openList.substr(0, pos + 1);
}

static std::string GetFilename(const std::string& openList)
{
  size_t pos = openList.find_last_of('/');
  auto fileWithExtension = pos == openList.npos ? openList : openList.substr(pos + 1, openList.npos);

  pos = fileWithExtension.find_last_of('.');
  return pos == openList.npos ? fileWithExtension : fileWithExtension.substr(0, pos);
}

static std::string GetDate()
{
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);

  std::ostringstream oss;
  oss << std::put_time(&tm, "_%Y%m%d_%H%M%S");

  return oss.str();
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

    if (t > 0 && t < 1 && u >= 0) {
      float x = x1 + t * (x2 - x1);
      float y = y1 + t * (y2 - y1);

      float dis = Distance(x3, y3, x, y);
      if (dis < minDistance)
      {
        minDistance = dis;
        minPoint = Point(x, y);
      }
    }
  }

  return minPoint;
}

static Point ToWorld(const Point& p)
{
  return Point(floor(p.x / TILE_SIZE), floor(p.y / TILE_SIZE));
}

static Point FromWorld(const Point& p)
{
  return Point(p.x * TILE_SIZE + HALF_TILE - 1, p.y * TILE_SIZE + HALF_TILE - 1);
}

static std::vector<Point> ToPoints(const std::vector<std::vector<Cost>>& map, const Point& start, const Point& dest)
{
  std::vector<Point> points;

  int x = dest.x;
  int y = dest.y;

  // Go back until we reach the start position
  for (int k = 0; k < 100; k++)
  {
    points.push_back(FromWorld(Point(x, y)));
    if (x == start.x && y == start.y)
      break;

    int tx = map[x][y].parent.x;
    int ty = map[x][y].parent.y;

    x = tx;
    y = ty;
  }

  std::reverse(points.begin(), points.end());

  return points;
}

static std::vector<Point> PathFinding(const Point& start, const Point& end, const std::vector<Line>& lines)
{
  Point cp = ToWorld(start);
  Point cpEnd = ToWorld(end);

  // Vector with nodes we still need to visit and costs were already calculated
  std::vector<Cost> openList;

  // Vector with all visited nodes
  std::vector<Point> closedList;

  const int width = WIDTH / TILE_SIZE;
  const int height = HEIGHT / TILE_SIZE;

  std::vector<std::vector<Cost>> map(width + 1, std::vector<Cost>(height + 1));

  // printf("Finding openList from %.2f %.2f to %.2f %.2f in map %ux%u\n", cp.x, cp.y, cpEnd.x, cpEnd.y, width, height);

  if (cpEnd.x < 0 || cpEnd.x >= width || cpEnd.y < 0 || cpEnd.y >= height)
    return std::vector<Point>();

  // Create map
  for (int i = 0; i <= width; ++i)
  {
    for (int j = 0; j <= height; ++j)
      map[i][j] = Cost(Point(i, j), Point(i, j));
  }

  {
    int cost = Distance(cp.x, cp.y, cpEnd.x, cpEnd.y);
    openList.push_back(Cost(cp, cp, 1, cost, 1 + cost));
    map[cp.x][cp.y] = Cost(cp, cp, 1, cost, 1 + cost);
  }

  // Limit number of trials so it doesn't take too long
  while (!openList.empty())
  {
    // Find smallest cost
    auto iter = openList.begin();
    for (auto i = openList.begin(); i < openList.end(); i++)
    {
      if (i->f < iter->f)
        iter = i;
    }

    Point p = iter->p;
    int cost = iter->g;

    // Remove from open list and add to the closed list
    openList.erase(iter);
    closedList.push_back(p);

    // printf("Selected %.2f %.2f - %d out of %lu - %lu items\n", p.x, p.y, p.f, openList.size() + 1, closedList.size());
    for (int i = -1; i <= 1; ++i)
    {
      for (int j = -1; j <= 1; ++j)
      {
        if (i == 0 && j == 0)
          continue;

        Point pp = Point(p.x + i, p.y + j);
        // printf("Check neighbour %.2f %.2f\n", pp.x, pp.y);
        if (pp.x < 0 || pp.x > width || pp.y < 0 || pp.y > height)
          continue;

        // Do not add neighbour if it is already in the closed list
        if (std::find_if(closedList.begin(), closedList.end(), [&pp](const Point& p){ return pp == p; }) != closedList.end())
          continue;

        // Check if we are blocked by a wall
        Point wp = FromWorld(p);
        Point wpp = FromWorld(pp);
        Point minPoint = Raycast(wp, wpp, lines);
        if (minPoint != wpp)
          continue;

        if (pp == cpEnd)
        {
          // Only the parent information is needed for the final location
          // printf("FOUND PATH\n");
          map[pp.x][pp.y].parent = p;
          return ToPoints(map, cp, cpEnd);
        }

        // Calculate costs
        int g = cost + Distance(pp.x, pp.y, p.x, p.y);
        int h = Distance(pp.x, pp.y, cpEnd.x, cpEnd.y);

        // Check if the new cost is cheaper than the one current in the map
        if (map[pp.x][pp.y].f <= g + h)
          continue;

        // The new cheaper cost should override the previous one
        std::remove_if(openList.begin(), openList.end(), [&pp](const Cost& c){ return pp == c.p; });

        openList.push_back(Cost(pp, p, g, h, g + h));
        map[pp.x][pp.y] = Cost(pp, p, g, h, g + h);

        // printf("Added neighbour %.2f %.2f - %.2f %.2f - %d\n", pp.x, pp.y, p.p.x, p.p.y, g + h);
      }
    }
  }

  // If openList is not found, return empty vector
  return std::vector<Point>();
  // For debugging, it is easier to see the full explored closedList
  // return closedList;
}