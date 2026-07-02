#include "WindowManager.hpp"
#include "Colors.hpp"
#include <box2d/box2d.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

WindowManager::WindowManager(const char* windowName, SDL_Color backgroundColor)
    : backgroundColor(backgroundColor) {
    size = WindowDimensions{1280, 720};
    sdlWindow =
        SDL_CreateWindow(windowName, size.x, size.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
    if (!sdlWindow) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Unable to create SDL Window: %s", SDL_GetError()
        );
        return;
    }
    SDL_Log("Created SDL Window with name \"%s\"", windowName);
    sdlRenderer = SDL_CreateRenderer(sdlWindow, nullptr);
    if (!sdlRenderer) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Unable to create SDL renderer: %s", SDL_GetError());
        return;
    }
    SDL_Log("Created SDL renderer");
    setVsync(vsync);
    setTargetFps(240);
    ImGui_ImplSDL3_InitForSDLRenderer(sdlWindow, sdlRenderer);
    ImGui_ImplSDLRenderer3_Init(sdlRenderer);
}

WindowManager::~WindowManager() {
    cleanup();
}

void WindowManager::cleanup() {
    if (sdlRenderer) {
        SDL_DestroyRenderer(sdlRenderer);
        sdlRenderer = nullptr;
        SDL_Log("Destroyed SDL renderer");
    }
    if (sdlWindow) {
        SDL_DestroyWindow(sdlWindow);
        sdlWindow = nullptr;
        SDL_Log("Destroyed SDL window");
    }
}

void WindowManager::render(Uint64 frameStartNs) {
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), sdlRenderer);
    SDL_RenderPresent(sdlRenderer);
    if (fpsUnlimited || vsync) {
        return;
    }
    const Uint64 frameTimeNs = SDL_GetTicksNS() - frameStartNs;
    if (frameTimeNs < targetFrameTimeNs) {
        const Uint64 delayNs = targetFrameTimeNs - frameTimeNs;
        SDL_DelayPrecise(delayNs);
    }
}

void WindowManager::clearFrame() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    SDL_SetRenderDrawColor(
        sdlRenderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a
    );
    SDL_RenderClear(sdlRenderer);
}

SDL_Window* WindowManager::getSdlWindow() const {
    return sdlWindow;
}

SDL_Renderer* WindowManager::getSdlRenderer() const {
    return sdlRenderer;
};

WindowDimensions WindowManager::getSizePixels() const {
    return size;
}

b2Vec2 WindowManager::getSizeWorld() const {
    return b2Vec2{size.x / scaleFactor, size.y / scaleFactor};
}

WindowDimensions WindowManager::getOffsetPixels() const {
    return offsetPixels;
}

b2Vec2 WindowManager::getOffsetWorld() const {
    return offsetWorld;
}

float WindowManager::getScaleFactor() const {
    return scaleFactor;
}

bool WindowManager::getIsFullscreen() const {
    return isFullscreen;
}

Uint64 WindowManager::getTargetFps() const {
    return targetFps;
}

std::string WindowManager::targetFpsStr() const {
    if (vsync) {
        SDL_DisplayID displayID = SDL_GetDisplayForWindow(sdlWindow);
        if (displayID == 0) {
            return "Unknown";
        }
        const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(displayID);
        if (!mode) {
            return "Unknown";
        }
        return std::to_string(static_cast<int>(SDL_roundf(mode->refresh_rate))) + ".0";
    } else if (fpsUnlimited) {
        return "Unlimited";
    } else {
        return std::to_string(targetFps) + ".0";
    }
}

void WindowManager::setTargetFps(Uint64 value) {
    targetFps = value;
    targetFrameTimeNs = 1000000000ULL / targetFps;
    SDL_Log("Set target fps to %zu", value);
}

void WindowManager::setVsync(bool value) {
    if (value && fpsUnlimited) {
        setFpsUnlimited(false);
    }
    int arg = value == true ? 1 : 0;
    if (!SDL_SetRenderVSync(sdlRenderer, arg)) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to toggle VSync: %s", SDL_GetError());
        return;
    }
    SDL_Log("Vsync set to %s", value ? "true" : "false");
    vsync = value;
}

bool WindowManager::isVsyncEnabled() const {
    return vsync;
}

void WindowManager::setFpsUnlimited(bool value) {
    if (vsync && value) {
        setVsync(false);
    }
    fpsUnlimited = value;
    SDL_Log("FPS Unlimited set to %s", value ? "true" : "false");
}
bool WindowManager::getFpsUnlimited() const {
    return fpsUnlimited;
}

void WindowManager::toggleFullscreen() {
    isFullscreen = !isFullscreen;
    SDL_SetWindowFullscreen(sdlWindow, isFullscreen);
}

void WindowManager::updateScaleFactor() {
    int dividend = b2MinInt(size.x, size.y);
    scaleFactor = dividend / 20.f * scaleMultiplier;
}

void WindowManager::updateOffset(std::optional<b2Vec2> worldPosition) {
    if (worldPosition.has_value()) {
        offsetWorld = worldPosition.value();
        offsetWorld.x = -offsetWorld.x;
    }
    offsetPixels.x = static_cast<int>(SDL_roundf(offsetWorld.x * scaleFactor));
    offsetPixels.y = static_cast<int>(SDL_roundf(offsetWorld.y * scaleFactor));

    offsetPixels.x += size.x / 2;
    offsetPixels.y += size.y / 2;
}

void WindowManager::handleResize(int x, int y) {
    size.x = x;
    size.y = y;
    updateScaleFactor();
    updateOffset(std::nullopt);
}

void WindowManager::incrementScaleMultiplierBy(float amount) {
    if (scaleMultiplier + amount <= 0) {
        return;
    }
    scaleMultiplier += amount;
    updateScaleFactor();
}

void WindowManager::resetScaleMultiplier() {
    scaleMultiplier = 1.f;
    updateScaleFactor();
}
