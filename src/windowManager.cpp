#include "windowManager.hpp"
#include "colors.hpp"
#include "config.hpp"
#include <box2d/box2d.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

WindowManager::WindowManager(void) {
    sdlWindow = SDL_CreateWindow(PLAT_WINDOW_NAME, 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
    sdlRenderer = SDL_CreateRenderer(sdlWindow, nullptr);
    SDL_SetRenderVSync(sdlRenderer, 1);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(sdlWindow, sdlRenderer);
    ImGui_ImplSDLRenderer3_Init(sdlRenderer);
}

WindowDimensions WindowManager::getSize(void) { return WindowDimensions{sizeX, sizeY}; }

WindowDimensions WindowManager::getOffset(void) { return WindowDimensions{offsetX, offsetY}; }

int WindowManager::getScaleFactor(void) { return scaleFactor; }

void WindowManager::handleResize(int x, int y) {
    sizeX = x;
    sizeY = y;
    offsetX = x / 2;
    offsetY = y;
    int dividend = b2MinInt(x, y);
    scaleFactor = dividend / 20;
}

void WindowManager::clearFrame(void) {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    SDL_SetRenderDrawColor(sdlRenderer, Colors.BackGround.r, Colors.BackGround.g, Colors.BackGround.b,
                           Colors.BackGround.a);
    SDL_RenderClear(sdlRenderer);
}