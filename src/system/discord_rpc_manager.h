#pragma once
#include <SDL3/SDL.h>
#include <string_view>
#ifdef USE_DISCORD_RPC
#include <discord_rpc.h>
#endif

namespace discord_rpc_manager {
#ifdef USE_DISCORD_RPC
void init(std::string_view application_id, DiscordRichPresence presence);
#endif
void update_state(std::string_view state, std::string_view details);
void update();
void shutdown();
}