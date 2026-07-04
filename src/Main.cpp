#include "DiscordRpcManager.hpp"
#include "FormatLogs.hpp"
#include "Platformer.hpp"
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

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
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    DiscordRpcManager::init();
    DiscordRpcManager::setStatus("In Development", nullptr);
    int returnVal = 0;
    try {
        Platformer game;
        game.run();
    } catch (const std::exception& e) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", e.what(), NULL);
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Fatal exception caught: %s", e.what());
        returnVal = 1;
    }
    DiscordRpcManager::shutdown();
    ImGui_ImplSDLRenderer3_Shutdown();
    SDL_Log("Shutdown ImGui SDL3 renderer implementation");
    ImGui_ImplSDL3_Shutdown();
    SDL_Log("Shutdown ImGui SDL3 implementation");
    ImGui::DestroyContext();
    SDL_Log("Destroyed ImGui context");
    TTF_Quit();
    SDL_Log("Quit SDL_ttf");
    SDL_Quit();
    SDL_Log("Quit SDL");
    return returnVal;
}