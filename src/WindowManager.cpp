#include "windowManager.hpp"
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
    sdlRenderer = SDL_CreateRenderer(sdlWindow, nullptr);
    SDL_SetRenderVSync(sdlRenderer, 1);
    ImGui_ImplSDL3_InitForSDLRenderer(sdlWindow, sdlRenderer);
    ImGui_ImplSDLRenderer3_Init(sdlRenderer);
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

void WindowManager::toggleFullscreen() {
    isFullscreen = !isFullscreen;
    SDL_SetWindowFullscreen(sdlWindow, isFullscreen);
}

WindowDimensions WindowManager::getSizePixels() const { return size; }

b2Vec2 WindowManager::getSizeWorld() const {
    return b2Vec2{size.x / scaleFactor, size.y / scaleFactor};
}

WindowDimensions WindowManager::getOffsetPixels() const { return offsetPixels; }

b2Vec2 WindowManager::getOffsetWorld() const { return offsetWorld; }

float WindowManager::getScaleFactor() const { return scaleFactor; }

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
