#pragma once
#include <SDL3/SDL.h>
#include <string_view>
#ifdef USE_DISCORD_RPC
#include <discord_rpc.h>
#endif

namespace DiscordRpcManager {
#ifdef USE_DISCORD_RPC
void init(std::string_view applicationId, DiscordRichPresence presence);
#endif
void updateState(std::string_view state, std::string_view details);
void update();
void shutdown();
}