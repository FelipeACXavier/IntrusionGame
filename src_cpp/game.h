#pragma once

#include <string>
#include <memory>

#include <nlohmann/json.hpp>
#include <SDL2/SDL.h> /* macOS- and GNU/Linux-specific */
#include "SDL2/SDL_ttf.h"

#include "level.h"

class Game
{
public:
  Game(const nlohmann::json& config);
  ~Game();

  bool Init(Args overrides);
  bool Run();

  bool Reset();

  Result GetResult();

private:
  bool mRun;
  bool mResult;

  SDL_Window *mWindow;
  SDL_Renderer *mRenderer;
  SDL_Texture *mTexture, *mText;
  TTF_Font* mFont;
  SDL_Rect mTextRect;

  nlohmann::json mConfig;

  std::unique_ptr<Level> mLevel;

  uint32_t mTicks;
  uint32_t mTotalTicks;

  bool SetupSDL(bool hidden);
  bool SetupGlobals(Args overrides);

  void Finished(bool result);
  bool IsDayDone() const;
  bool IsDone() const;
  void RenderUI();
};