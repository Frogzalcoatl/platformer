#include "system/discord_rpc_manager.h"
#ifdef USE_DISCORD_RPC
#include <SDL3/SDL.h>
#include <ctime>
#include <string>

static time_t last_connection_attempt = 0;
static const int reconnect_interval_seconds = 5;
static DiscordEventHandlers handlers{};
static DiscordRichPresence presence{};
static bool is_connected = false;
static std::string application_id = "";

static std::string presence_state = "";
static std::string presence_details = "";

static void handle_discord_ready(const DiscordUser* connected_user) {
    (void)connected_user;
    is_connected = true;
    SDL_Log("Discord RPC connected");
    discord_rpc_manager::update_state(presence_state, presence_details);
}

static void handle_discord_disconnected(int error_code, const char* message) {
    is_connected = false;
    SDL_Log("Discord RPC disconnected - Error %d: %s", error_code, message);
}

static void discord_connect() {
    Discord_Initialize(application_id.c_str(), &handlers, 1, nullptr);
    presence.startTimestamp = time(nullptr);
}

void discord_rpc_manager::init(
    std::string_view application_id_arg, DiscordRichPresence presence_arg
) {
    handlers.ready = handle_discord_ready;
    handlers.disconnected = handle_discord_disconnected;
    handlers.errored = handle_discord_disconnected;
    application_id = application_id_arg;
    presence_state = presence_arg.state ? presence_arg.state : "";
    presence_details = presence_arg.details ? presence_arg.details : "";
    presence.state = presence_state.c_str();
    presence.details = presence_details.c_str();
    presence = presence_arg;
    discord_connect();
    SDL_Log("Initialized DiscordRpcManager");
}

void discord_rpc_manager::update_state(std::string_view state, std::string_view details) {
    presence_state = state;
    presence_details = details;
    presence.state = presence_state.c_str();
    presence.details = presence_details.c_str();
    Discord_UpdatePresence(&presence);
    if (is_connected) {
        SDL_Log(
            "Updated Discord RPC status - State: \"%s\" | Details: \"%s\"",
            presence.state,
            presence_details.c_str()
        );
    }
}

void discord_rpc_manager::update() {
    if (!is_connected) {
        time_t now = time(nullptr);
        if ((now - last_connection_attempt) > reconnect_interval_seconds) {
            last_connection_attempt = now;
            discord_connect();
        }
    }
    Discord_RunCallbacks();
}

void discord_rpc_manager::shutdown() {
    Discord_Shutdown();
    SDL_Log("Shutdown Discord RPC");
}
#else
void discord_rpc_manager::update_state(std::string_view state, std::string_view details) {
    (void)state;
    (void)details;
}

void discord_rpc_manager::update() {
}

void discord_rpc_manager::shutdown() {
}
#endif