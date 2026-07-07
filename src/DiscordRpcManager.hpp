#pragma once
#include <ctime>
#include <discord_rpc.h>
#include <string>

namespace DiscordRpcManager {
void init();
void setStatus(const char* state, const char* details);
void update();
void shutdown();
}