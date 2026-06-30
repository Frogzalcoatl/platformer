#include "FormatLogs.hpp"
#include "Platformer.hpp"
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <imgui.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    initLogFormatting();
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL Initialization failed: %s", SDL_GetError());
        return 1;
    }
    SDL_Log("Initialized SDL with flags SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO");
    if (!TTF_Init()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf Initialization failed: %s", SDL_GetError()
        );
        return 1;
    }
    SDL_Log("Initialized SDL_ttf");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    SDL_Log("Created ImGui Context");
    Platformer game;
    game.run();
    game.close();
    return 0;
}