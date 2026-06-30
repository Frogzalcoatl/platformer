#include "FormatLogs.hpp"
#include "Platformer.hpp"
#include <SDL3_ttf/SDL_ttf.h>
#include <imgui.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    initLogFormatting();
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
        SDL_Log("SDL Initialization failed: %s", SDL_GetError());
        return 1;
    }
    if (!TTF_Init()) {
        SDL_Log("SDL_ttf Initialization failed: %s", SDL_GetError());
        return 1;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    Platformer game;
    game.run();
    game.close();
    return 0;
}