#include "colors.hpp"
#include "debugUi.hpp"
#include "physicsStep.hpp"
#include "player.hpp"
#include "windowManager.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <box2d/box2d.h>
#include <vector>

static std::vector<GameObject*> gameObjects = {};

void addTestObjects(b2WorldId world) {
    gameObjects.push_back(
        new GameObject{world, {50.f, 0.5f}, {0.f, 0.5f}, std::nullopt, std::nullopt, Colors.GrassGreen});
    gameObjects.push_back(new GameObject{world, {0.5f, 10.f}, {-18.f, 11.f}, std::nullopt, std::nullopt, Colors.Gray});
    gameObjects.push_back(new GameObject{world, {0.5f, 10.f}, {18.f, 11.f}, std::nullopt, std::nullopt, Colors.Gray});
    b2BodyDef logBodyDef = b2DefaultBodyDef();
    logBodyDef.type = b2_dynamicBody;
    logBodyDef.linearDamping = 0.5f;
    gameObjects.push_back(new GameObject{world, {0.5f, 2.f}, {10.f, 3.f}, logBodyDef, std::nullopt, Colors.Brown});
    gameObjects.push_back(new GameObject{world, {0.5f, 1.f}, {-10.f, 2.f}, logBodyDef, std::nullopt, Colors.Brown});
}

void shutDown(b2WorldId world, WindowManager* window) {
    b2DestroyWorld(world);
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(window->sdlRenderer);
    SDL_DestroyWindow(window->sdlWindow);
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        SDL_Log("SDL Initialization failed: %s", SDL_GetError());
        return -1;
    }
    WindowManager window{};
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -22.0f};
    b2WorldId world = b2CreateWorld(&worldDef);
    addTestObjects(world);
    Player player(world, {0.5f, 0.5f}, {0.f, 4.f}, Colors.Purple);
    lastTime = SDL_GetTicks();
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                window.handleResize(event.window.data1, event.window.data2);
                continue;
            }
            player.handleSDLKeyEvent(&event);
        }
        handlePhysicsStep(world, &player);
        window.clearFrame();
        player.draw(&window);
        drawUI(&window, &player);
        for (GameObject* object : gameObjects) {
            object->draw(&window);
        }
        SDL_RenderPresent(window.sdlRenderer);
    }
    shutDown(world, &window);
    return 0;
}