#include "DiscordRpcManager.hpp"
#include <SDL3/SDL.h>

static time_t lastConnectionAttempt = 0;
static const int ReconnectIntervalSeconds = 5;
static DiscordEventHandlers handlers;
static DiscordRichPresence presence;
static bool isConnected = false;
static std::string applicationId = "";

static void handleDiscordReady(const DiscordUser* connectedUser) {
    (void)connectedUser;
    isConnected = true;
    SDL_Log("Discord RPC connected");
    DiscordRpcManager::updatePresence(presence);
}

static void handleDiscordDisconnected(int errorCode, const char* message) {
    isConnected = false;
    SDL_Log("Discord RPC disconnected - Error %d: %s", errorCode, message);
}

static void discordConnect() {
    Discord_Initialize(applicationId.c_str(), &handlers, 1, nullptr);
    presence.startTimestamp = time(nullptr);
}

void DiscordRpcManager::init(std::string_view applicationIdArg, DiscordRichPresence& presenceArg) {
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.errored = handleDiscordDisconnected;
    applicationId = applicationIdArg;
    presence = presenceArg;
    discordConnect();
}

void DiscordRpcManager::updatePresence(DiscordRichPresence& presenceArg) {
    presence = presenceArg;
    Discord_UpdatePresence(&presence);
    if (isConnected) {
        SDL_Log("Updated Discord RPC");
    }
}

void DiscordRpcManager::updateState(const char* message) {
    presence.state = message;
    Discord_UpdatePresence(&presence);
    if (isConnected) {
        SDL_Log("Updated Discord RPC status: \"%s\"", presence.state);
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