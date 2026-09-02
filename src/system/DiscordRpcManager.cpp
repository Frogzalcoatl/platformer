#include "system/DiscordRpcManager.hpp"
#ifdef USE_DISCORD_RPC
#include <SDL3/SDL.h>
#include <ctime>
#include <string>


static time_t lastConnectionAttempt = 0;
static const int ReconnectIntervalSeconds = 5;
static DiscordEventHandlers handlers{};
static DiscordRichPresence presence{};
static bool isConnected = false;
static std::string applicationId = "";

static std::string presenceState = "";
static std::string presenceDetails = "";

static void handleDiscordReady(const DiscordUser* connectedUser) {
    (void)connectedUser;
    isConnected = true;
    SDL_Log("Discord RPC connected");
    DiscordRpcManager::updateState(presenceState, presenceDetails);
}

static void handleDiscordDisconnected(int errorCode, const char* message) {
    isConnected = false;
    SDL_Log("Discord RPC disconnected - Error %d: %s", errorCode, message);
}

static void discordConnect() {
    Discord_Initialize(applicationId.c_str(), &handlers, 1, nullptr);
    presence.startTimestamp = time(nullptr);
}

void DiscordRpcManager::init(std::string_view applicationIdArg, DiscordRichPresence presenceArg) {
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.errored = handleDiscordDisconnected;
    applicationId = applicationIdArg;
    presenceState = presenceArg.state ? presenceArg.state : "";
    presenceDetails = presenceArg.details ? presenceArg.details : "";
    presence.state = presenceState.c_str();
    presence.details = presenceDetails.c_str();
    presence = presenceArg;
    discordConnect();
    SDL_Log("Initialized DiscordRpcManager");
}

void DiscordRpcManager::updateState(std::string_view state, std::string_view details) {
    presenceState = state;
    presenceDetails = details;
    presence.state = presenceState.c_str();
    presence.details = presenceDetails.c_str();
    Discord_UpdatePresence(&presence);
    if (isConnected) {
        SDL_Log(
            "Updated Discord RPC status - State: \"%s\" | Details: \"%s\"",
            presence.state,
            presenceDetails.c_str()
        );
    }
}

void DiscordRpcManager::update() {
    if (!isConnected) {
        time_t now = time(nullptr);
        if ((now - lastConnectionAttempt) > ReconnectIntervalSeconds) {
            lastConnectionAttempt = now;
            discordConnect();
        }
    }
    Discord_RunCallbacks();
}

void DiscordRpcManager::shutdown() {
    Discord_Shutdown();
    SDL_Log("Shutdown Discord RPC");
}
#else
void DiscordRpcManager::updateState(std::string_view state, std::string_view details) {
    (void)state;
    (void)details;
}

void DiscordRpcManager::update() {
}

void DiscordRpcManager::shutdown() {
}
#endif