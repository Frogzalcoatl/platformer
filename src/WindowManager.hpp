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
    WindowDimensions offsetPixels;
    b2Vec2 offsetWorld = {0.f, 0.f};
    float scaleFactor = 1.f;
    bool isFullscreen = false;
    Uint64 targetFps = 240;
    Uint64 targetFrameTimeNs = 1000000000ULL / targetFps;
    bool vsync = false;
    SDL_Renderer* sdlRenderer;
    SDL_Window* sdlWindow;

    void updateScaleFactor();

  public:
    float scaleMultiplier = 1.f;
    SDL_Color backgroundColor;
    bool unlimitedFps = false;

    WindowManager(const char* windowName, SDL_Color backgroundColor);

    void cleanup();

    SDL_Window* getSdlWindow() const;
    SDL_Renderer* getSdlRenderer() const;
    WindowDimensions getSizePixels() const;
    b2Vec2 getSizeWorld() const;
    WindowDimensions getOffsetPixels() const;
    b2Vec2 getOffsetWorld() const;
    float getScaleFactor() const;
    bool getIsFullscreen() const;
    Uint64 getTargetFps() const;
    std::string targetFpsStr() const;
    void setTargetFps(Uint64 value);
    void setVsync(bool value);
    bool isVsyncEnabled() const;

    void render(Uint64 frameStartNs);
    void clearFrame();
    void toggleFullscreen();
    void handleResize(int sizeX, int sizeY);
    void incrementScaleMultiplierBy(float amount);
    void resetScaleMultiplier();
    void updateOffset(std::optional<b2Vec2> worldPosition);
};