#include "platformer/Platformer.hpp"
#include "system/DiscordRpcManager.hpp"
#include "system/LogFormatting.hpp"
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    initSdlLogFormatting();
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
    if (!ImGui::CreateContext()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create ImGui Context.");
        return 1;
    }
    SDL_Log("Created ImGui Context");
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    io.ConfigNavSwapGamepadButtons = true; // Matches my controller. Will enable/disable this based
                                           // on controller type properly in the future
#ifdef USE_DISCORD_RPC
    // https://discord.com/developers/applications/1521649642360668300/
    DiscordRichPresence presence = {};
    presence.largeImageKey = "icon";
    presence.largeImageText = "Platformer";
    presence.state = "In Development";
    DiscordRpcManager::init("1521649642360668300", presence);
#endif
    int returnVal = 0;
    try {
        Platformer game;
        game.run();
    } catch (const std::exception& e) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", e.what(), NULL);
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Fatal exception caught: %s", e.what());
        returnVal = 1;
    }
#ifdef USE_DISCORD_RPC
    DiscordRpcManager::shutdown();
#endif
    ImGui_ImplSDLRenderer3_Shutdown();
    SDL_Log("Shutdown ImGui SDL3 renderer implementation");
    ImGui_ImplSDL3_Shutdown();
    SDL_Log("Shutdown ImGui SDL3 implementation");
    ImGui::DestroyContext();
    SDL_Log("Destroyed ImGui context");
    MIX_Quit();
    SDL_Log("Quit SDL3_mixer");
    TTF_Quit();
    SDL_Log("Quit SDL3_ttf");
    SDL_Quit();
    SDL_Log("Quit SDL3");
    return returnVal;
}