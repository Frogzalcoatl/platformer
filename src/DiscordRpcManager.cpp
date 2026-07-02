#include "DiscordRpcManager.hpp"
#include <SDL3/SDL.h>

static time_t lastConnectionAttempt = 0;
static const int ReconnectIntervalSeconds = 10;
static DiscordEventHandlers handlers;
static DiscordRichPresence presence;
static bool isConnected = false;
static const char* ApplicationId = "1521649642360668300";
// https://discord.com/developers/applications/1521649642360668300/

static void handleDiscordReady(const DiscordUser* connectedUser) {
    (void)connectedUser;
    isConnected = true;
    SDL_Log("Discord RPC connected");
    DiscordRpcManager::setStatus(presence.state, presence.details);
}

static void handleDiscordDisconnected(int errorCode, const char* message) {
    (void)errorCode;
    (void)message;
    isConnected = false;
    SDL_Log("Discord RPC disconnected");
}

static void discordConnect() {
    Discord_Initialize(ApplicationId, &handlers, 1, nullptr);
    presence.startTimestamp = time(nullptr);
}

void DiscordRpcManager::init() {
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.errored = handleDiscordDisconnected;
    discordConnect();
}

void DiscordRpcManager::setStatus(const char* state, const char* details) {
    presence.state = state;
    presence.details = details;
    presence.largeImageKey = "icon";
    presence.largeImageText = "Platformer";
    Discord_UpdatePresence(&presence);
    SDL_Log(
        "Set Discord RPC status: \"%s\" | \"%s\" | \"%s\"",
        presence.largeImageText,
        presence.state,
        presence.details
    );
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