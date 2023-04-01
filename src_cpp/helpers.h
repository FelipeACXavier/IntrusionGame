#pragma once

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
