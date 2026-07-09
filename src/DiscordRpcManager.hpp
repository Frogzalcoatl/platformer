#pragma once
#include <ctime>
#include <discord_rpc.h>
#include <string>

namespace DiscordRpcManager {
void init(std::string_view applicationId, DiscordRichPresence& presence);
void updatePresence(DiscordRichPresence& presenceArg);
void updateState(const char* message);
void update();
void shutdown();
}