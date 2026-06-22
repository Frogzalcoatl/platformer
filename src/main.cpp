#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <box2d/box2d.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        SDL_Log("SDL Initialization failed: %s", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("C++ Platformer", 1280, 720, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Initialize Box2D (Using v3.0 C-style API as an example, adjust if using v2.4 C++ classes)
    b2WorldDef worldDef = b2DefaultWorldDef(); // Initializes defaults
    worldDef.gravity = {0.0f, -9.8f};          // Overwrite gravity
    b2WorldId world = b2CreateWorld(&worldDef); // Cleanly pass the address

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Start UI Frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // 1. Step Physics
        b2World_Step(world, 1.0f / 60.0f, 4);

        // 2. Render Game Logic (Use standard C++ container loops for DSA practice)
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255); // Dark blue background
        SDL_RenderClear(renderer);

        // 3. Simple UI Overlays
        ImGui::Begin("Debug Menu");
        ImGui::Text("Refresh C++ Syntax & DSA");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    b2DestroyWorld(world);
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}