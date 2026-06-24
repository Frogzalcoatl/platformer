#pragma once
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

struct WindowDimensions {
    int x;
    int y;
};

class WindowManager {
  private:
    int sizeX;
    int sizeY;
    int offsetX;
    int offsetY;
    int scaleFactor;

  public:
    SDL_Renderer* sdlRenderer;
    SDL_Window* sdlWindow;

    WindowManager(void);

    WindowDimensions getSize(void);
    WindowDimensions getOffset(void);
    int getScaleFactor(void);
    void handleResize(int sizeX, int sizeY);
    void clearFrame(void);
};