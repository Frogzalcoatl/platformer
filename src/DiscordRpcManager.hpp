#pragma once
#include <SDL3/SDL.h>
#include <ctime>
#include <string>
#if defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_MACOS) || defined(SDL_PLATFORM_LINUX)
#define DISCORD_RPC_MANAGER 1
#include <discord_rpc.h>
#endif

namespace DiscordRpcManager {
#ifdef DISCORD_RPC_MANAGER
void init(std::string_view applicationId, DiscordRichPresence presence);
#endif
void updateState(std::string_view state, std::string_view details);
void update();
void shutdown();
}