#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <optional>
#include <string>

struct WindowVec2 {
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
    UniqueWindow sdlWindow;
    UniqueRenderer sdlRenderer;
    WindowVec2 size;
    WindowVec2 mousePos;
    Uint64 targetFps = 240;
    Uint64 targetFrameTimeNs = 1000000000ULL / targetFps;
    bool vsync = true;
    bool fpsUnlimited = false;
    bool isFullscreen = false;

  public:
    SDL_Color backgroundColor;

    WindowManager(const char* windowName, SDL_Color backgroundColor);

    void clearFrame();

    void render(Uint64 frameStartNs);

    SDL_Window* getSdlWindow() const;

    SDL_Renderer* getSdlRenderer() const;

    WindowVec2 getSize() const;

    void handleResize(int sizeX, int sizeY);

    WindowVec2 getMousePos() const;

    void handleMouseMotionEvent(const SDL_MouseMotionEvent& event);

    Uint64 getTargetFps() const;

    float getVsyncFps() const;

    std::string targetFpsStr() const;

    void setTargetFps(Uint64 value);

    bool isVsyncEnabled() const;

    void setVsync(bool value);

    bool getFpsUnlimited() const;

    void setFpsUnlimited(bool value);

    bool getIsFullscreen() const;

    void toggleFullscreen();
};