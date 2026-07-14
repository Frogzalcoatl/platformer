#pragma once
#include <SDL3/SDL.h>
#include <ctime>
#include <string>
#ifdef USE_DISCORD_RPC
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