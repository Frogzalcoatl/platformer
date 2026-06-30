#include "FormatLogs.hpp"
#include "Platformer.hpp"
#include <SDL3/SDL_main.h>
#include <imgui.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    initLogFormatting();
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "SDL3 Initialization failed: %s", SDL_GetError()
        );
        return 1;
    }
    SDL_Log("Initialized SDL3 with flags SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO");
    if (!TTF_Init()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "SDL3_ttf Initialization failed: %s", SDL_GetError()
        );
        return 1;
    }
    SDL_Log("Initialized SDL3_ttf");
    if (!MIX_Init()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "SDL3_mixer Initialization failed: %s", SDL_GetError()
        );
        return 1;
    }
    SDL_Log("Initialized SDL3_mixer");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    SDL_Log("Created ImGui Context");
    Platformer game;
    game.run();
    game.close();
    return 0;
}