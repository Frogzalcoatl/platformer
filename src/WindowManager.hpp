#pragma once
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <memory>
#include <optional>
#include <string>

struct WindowDimensions {
    int x;
    int y;
};

struct SDL_Window_Deleter {
    void operator()(SDL_Window* w) const {
        if (w) {
            SDL_DestroyWindow(w);
        }
    }
};
struct SDL_Renderer_Deleter {
    void operator()(SDL_Renderer* r) const {
        if (r) {
            SDL_DestroyRenderer(r);
        }
    }
};
using UniqueWindow = std::unique_ptr<SDL_Window, SDL_Window_Deleter>;
using UniqueRenderer = std::unique_ptr<SDL_Renderer, SDL_Renderer_Deleter>;

class WindowManager {
  private:
    WindowDimensions size;
    bool isFullscreen = false;
    Uint64 targetFps = 240;
    Uint64 targetFrameTimeNs = 1000000000ULL / targetFps;
    bool vsync = false;
    bool fpsUnlimited = false;
    UniqueRenderer sdlRenderer;
    UniqueWindow sdlWindow;

  public:
    SDL_Color backgroundColor;

    WindowManager(const char* windowName, SDL_Color backgroundColor);

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