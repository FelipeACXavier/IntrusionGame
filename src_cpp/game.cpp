#include "game.h"

#include <stdio.h>

#include "helpers.h"
#include "settings.h"

// Globals
bool HIDDEN;
uint32_t FPS;
uint32_t WIDTH;
uint32_t HEIGHT;
uint32_t TILE_SIZE;
uint32_t HALF_TILE;
uint32_t DAY_LENGTH;
uint32_t CYCLES_PER_FRAME;

SDL_Color UI_COLOR = { 255, 127, 80 };

Game::Game(const nlohmann::json& config)
    : mRun(true)
    , mResult(false)
    , mConfig(config)
    , mTicks(0)
    , mTotalTicks(0)
{
}

Game::~Game()
{
  // Clean up SDL pointers
  if (mFont)
    TTF_CloseFont(mFont);

  if (mText)
    SDL_DestroyTexture(mText);

  if (mTexture)
    SDL_DestroyTexture(mTexture);

  if (mRenderer)
    SDL_DestroyRenderer(mRenderer);

  if (mWindow)
    SDL_DestroyWindow(mWindow);

  TTF_Quit();
  SDL_Quit();
}

bool Game::Init(Args overrides)
{
  if (mWindow)
  {
    printf("Window already exists\n");
    return true;
  }

  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  // Initialise globals
  RETURN_ON_FAILURE(SetupGlobals(overrides));

  // Initialise SDL
  RETURN_ON_FAILURE(SetupSDL());

  return Reset();
}

bool Game::SetupGlobals(Args overrides)
{
  FPS              = overrides.fps > 0 ? overrides.fps : uint32_t(mConfig["fps"]);
  CYCLES_PER_FRAME = overrides.cycles > 0 ? overrides.cycles : uint32_t(mConfig["cycles_per_frame"]);
  DAY_LENGTH       = uint32_t(mConfig["day_duration"]);

  TILE_SIZE        = uint32_t(mConfig["tile_size"]);
  HALF_TILE        = TILE_SIZE / 2;

  WIDTH = float(mConfig["width"]) * TILE_SIZE;
  HEIGHT = float(mConfig["height"]) * TILE_SIZE;

  HIDDEN = overrides.hidden;

  mTotalTicks = DAY_LENGTH * 60 * 60;

  return true;
}

bool Game::SetupSDL()
{
  SDL_DisplayMode DM;
  SDL_GetCurrentDisplayMode(0, &DM);

  uint32_t flags = HIDDEN ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN;
  mWindow = SDL_CreateWindow("Intrusion game", (DM.w - WIDTH) / 2, (DM.h - HEIGHT) / 2, WIDTH, HEIGHT, flags);
  LOG_AND_RETURN_ON_FAILURE(mWindow, "Failed to create window");

  mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
  LOG_AND_RETURN_ON_FAILURE(mRenderer, "Failed to create window");

  mFont = TTF_OpenFont("/usr/share/fonts/fonts/ttf/JetBrainsMonoNL-SemiBold.ttf", 18);
  if (mFont == NULL)
    mFont = TTF_OpenFont("/usr/share/fonts/fonts/truetype/open-sans/OpenSans-Regular.ttf", 18);

  // Rectagle used to display UI
  mTextRect.x = 10;
  mTextRect.y = 10;

  return true;
}

bool Game::Reset()
{
  mRun = true;
  mResult = false;
  mTicks = 0;

  // (Re)Create level
  mLevel = std::make_unique<Level>(mRenderer);
  mLevel->mReachedDoor = [this]{ Finished(true); };
  mLevel->mWasCaught = [this]{ Finished(false); };

  return mLevel->Init(mConfig);
}

bool Game::Run()
{
  while (!IsDone())
  {
    SDL_Event event;
    while(SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT ||
          (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
      {
        Finished(false);
        return false;  // Return false to prevent future runs
      }
    }

    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
    SDL_RenderClear(mRenderer);

    for (uint32_t i = 0; !IsDone() && i < CYCLES_PER_FRAME; ++i)
    {
      if (IsDayDone())
      {
        Finished(true);
        break;
      }

      mLevel->Run();
      ++mTicks;
    }

    if (HIDDEN)
      continue;

    RenderUI();

    SDL_RenderPresent(mRenderer);
    SDL_Delay(1000 / FPS);
  }

  return true;
}

void Game::RenderUI()
{
  // Render texture
  SDL_RenderCopy(mRenderer, mTexture, NULL, NULL);

  std::string text = std::to_string(float(mTotalTicks - mTicks) / 60);
  SDL_Surface* text_surf = TTF_RenderText_Solid(mFont, text.c_str(), UI_COLOR);
  mText = SDL_CreateTextureFromSurface(mRenderer, text_surf);

  mTextRect.w = text_surf->w;
  mTextRect.h = text_surf->h;
  SDL_RenderCopy(mRenderer, mText, NULL, &mTextRect);
  SDL_FreeSurface(text_surf);
}

void Game::Finished(bool result)
{
  mRun = false;
  mResult = result;
}

Result Game::GetResult()
{
  return Result{mResult, float(mTicks) / 60, mLevel->GetResult()};
}

bool Game::IsDone() const
{
  return !mRun;
}

bool Game::IsDayDone() const
{
  return mTicks >= mTotalTicks;
}