#pragma once
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <optional>

struct WindowDimensions {
    int x;
    int y;
};

class WindowManager {
  private:
    WindowDimensions size;
    WindowDimensions offsetPixels;
    b2Vec2 offsetWorld = {0.f, 0.f};
    float scaleFactor;
    float scaleMultiplier = 1.f;
    bool isFullscreen = false;

    void updateScaleFactor(void);

  public:
    SDL_Renderer* sdlRenderer;
    SDL_Window* sdlWindow;

    WindowManager(void);

    void clearFrame(void);
    void toggleFullscreen(void);
    void handleResize(int sizeX, int sizeY);
    void incrementScaleMultiplierBy(float amount);
    void resetScaleMultiplier(void);
    void updateOffset(std::optional<b2Vec2> worldPosition);

    WindowDimensions getSizePixels(void);
    b2Vec2 getSizeWorld(void);
    WindowDimensions getOffsetPixels(void);
    b2Vec2 getOffsetWorld(void);
    float getScaleFactor(void);
    float getScaleMultiplier(void);
};