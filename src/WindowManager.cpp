#include "WindowManager.hpp"
#include "Colors.hpp"
#include <box2d/box2d.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

WindowManager::WindowManager(const char* windowName, SDL_Color backgroundColor)
    : backgroundColor(backgroundColor) {
    size = WindowVec2{1280, 720};
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED;
#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS)
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    flags |= SDL_WINDOW_FULLSCREEN;
#endif
    isFullscreen = ((flags & SDL_WINDOW_FULLSCREEN) != 0); // Returns true if fullscreen bit is 1
    sdlWindow = UniqueWindow(SDL_CreateWindow(windowName, size.x, size.y, flags));
    if (!sdlWindow) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Unable to create SDL3 Window: %s", SDL_GetError()
        );
        return;
    }
    SDL_Log("Created SDL3 Window with name \"%s\"", windowName);
    sdlRenderer = UniqueRenderer(SDL_CreateRenderer(sdlWindow.get(), nullptr));
    if (!sdlRenderer) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Unable to create SDL3 renderer: %s", SDL_GetError());
        return;
    }
    SDL_Log("Created SDL3 renderer");
    setVsync(vsync);
    Uint64 vsyncFps = static_cast<Uint64>(getVsyncFps());
    targetFps = vsyncFps;
    targetFrameTimeNs = 1000000000ULL / targetFps;
    ImGui_ImplSDL3_InitForSDLRenderer(sdlWindow.get(), sdlRenderer.get());
    ImGui_ImplSDLRenderer3_Init(sdlRenderer.get());
}

void WindowManager::clearFrame() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    SDL_SetRenderDrawColor(
        sdlRenderer.get(),
        backgroundColor.r,
        backgroundColor.g,
        backgroundColor.b,
        backgroundColor.a
    );
    SDL_RenderClear(sdlRenderer.get());
}

void WindowManager::render(Uint64 frameStartNs) {
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), sdlRenderer.get());
    SDL_RenderPresent(sdlRenderer.get());
    if (fpsUnlimited || vsync) {
        return;
    }
    const Uint64 frameTimeNs = SDL_GetTicksNS() - frameStartNs;
    if (frameTimeNs < targetFrameTimeNs) {
        const Uint64 delayNs = targetFrameTimeNs - frameTimeNs;
        SDL_DelayPrecise(delayNs);
    }
}

SDL_Window* WindowManager::getSdlWindow() const {
    return sdlWindow.get();
}

SDL_Renderer* WindowManager::getSdlRenderer() const {
    return sdlRenderer.get();
}

WindowVec2 WindowManager::getSize() const {
    return size;
}

void WindowManager::handleResize(int sizeX, int sizeY) {
    size.x = sizeX;
    size.y = sizeY;
}

WindowVec2 WindowManager::getMousePos() const {
    return mousePos;
}

void WindowManager::handleMouseMotionEvent(const SDL_MouseMotionEvent& event) {
    mousePos.x = static_cast<int>(event.x);
    mousePos.y = static_cast<int>(event.y);
}

Uint64 WindowManager::getTargetFps() const {
    return targetFps;
}

float WindowManager::getVsyncFps() const {
    SDL_DisplayID displayID = SDL_GetDisplayForWindow(sdlWindow.get());
    if (displayID == 0) {
        return 60;
    }
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(displayID);
    if (!mode) {
        return 60;
    }
    return mode->refresh_rate;
}

std::string WindowManager::targetFpsStr() const {
    if (vsync) {
        float vsyncFps = getVsyncFps();
        return std::to_string(static_cast<int>(SDL_roundf(vsyncFps))) + ".0";
    } else if (fpsUnlimited) {
        return "Unlimited";
    } else {
        return std::to_string(targetFps);
    }
}

void WindowManager::setTargetFps(Uint64 value) {
    // Logic is copied in constructor to avoid log
    targetFps = value;
    targetFrameTimeNs = 1000000000ULL / targetFps;
    SDL_Log("Set target fps to %zu", value);
}

bool WindowManager::isVsyncEnabled() const {
    return vsync;
}

void WindowManager::setVsync(bool value) {
    if (value && fpsUnlimited) {
        setFpsUnlimited(false);
    }
    int arg = value == true ? 1 : 0;
    if (!SDL_SetRenderVSync(sdlRenderer.get(), arg)) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to toggle VSync: %s", SDL_GetError());
        return;
    }
    SDL_Log("Vsync set to %s", value ? "true" : "false");
    vsync = value;
}

bool WindowManager::getFpsUnlimited() const {
    return fpsUnlimited;
}

void WindowManager::setFpsUnlimited(bool value) {
    if (vsync && value) {
        setVsync(false);
    }
    fpsUnlimited = value;
    SDL_Log("FPS Unlimited set to %s", value ? "true" : "false");
}

bool WindowManager::getIsFullscreen() const {
    return isFullscreen;
}

void WindowManager::toggleFullscreen() {
    isFullscreen = !isFullscreen;
    SDL_SetWindowFullscreen(sdlWindow.get(), isFullscreen);
}