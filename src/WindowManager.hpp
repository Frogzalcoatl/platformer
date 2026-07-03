#pragma once
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <optional>
#include <string>

struct WindowDimensions {
    int x;
    int y;
};

class WindowManager {
  private:
    WindowDimensions size;
    bool isFullscreen = false;
    Uint64 targetFps = 240;
    Uint64 targetFrameTimeNs = 1000000000ULL / targetFps;
    bool vsync = false;
    bool fpsUnlimited = false;
    SDL_Renderer* sdlRenderer;
    SDL_Window* sdlWindow;

  public:
    SDL_Color backgroundColor;

    WindowManager(const char* windowName, SDL_Color backgroundColor);
    ~WindowManager();

    void cleanup();

    SDL_Window* getSdlWindow() const;
    SDL_Renderer* getSdlRenderer() const;
    WindowDimensions getSize() const;
    bool getIsFullscreen() const;
    Uint64 getTargetFps() const;
    std::string targetFpsStr() const;
    void setTargetFps(Uint64 value);
    void setVsync(bool value);
    bool isVsyncEnabled() const;
    void setFpsUnlimited(bool value);
    bool getFpsUnlimited() const;
    void render(Uint64 frameStartNs);
    void clearFrame();
    void toggleFullscreen();
    void handleResize(int sizeX, int sizeY);
};