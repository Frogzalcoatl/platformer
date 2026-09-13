#pragma once
#include <SDL3/SDL.h>
#include <functional>
#include <string>
#include <variant>

enum class InputVerb : uint8_t {
    up,
    down,
    left,
    right,
    jump,
    sprint,
    respawn,
    confirm,
    cancel,
    pause,
    zoom_in,
    zoom_out,
    zoom_reset,
    toggle_fullscreen,
    toggle_debug,
    show_hitboxes,
    verb_count
};

struct InputVerbInfo {
    InputVerb verb;
    bool activate_on_repeat = false;
};

enum class InputState : uint8_t {
    pressed,
    released,
    input_state_count
};

enum class InputType : uint8_t {
    keyboard,
    mouse,
    controller,
    touch,
    input_type_count
};

struct DefaultScancodeBinding {
    InputVerb verb;
    SDL_Scancode scancode;
    bool activate_on_repeat = false;
};

struct DefaultButtonBinding {
    InputVerb verb;
    SDL_GamepadButton button;
};

struct InputSource {
    InputType type;
    Uint32 sdl_id;
    bool operator==(const InputSource& other) const = default;
};

enum class AudioCategory : uint8_t {
    master,
    sounds,
    music,
    audio_category_count
};

enum class UiState : uint8_t {
    main_menu,
    settings,
    player_source_setup,
    playing,
    paused,
    paused_settings,
    ui_state_count
};

enum class LevelName : uint8_t {
    none,
    test,
    level_name_count
};

enum class UserDataTypes : uint8_t {
    settings,
    user_data_types_count
};

namespace game_event_types {
struct CloseWindow {};

struct Input {
    InputVerb verb;
    InputState state;
    InputSource source_info;
};

struct PlaySound {
    std::string relative_path;
    unsigned int volume = 100;
    float pitch = 1.f;

    PlaySound(std::string_view path, unsigned int vol = 100, float p = 1.f)
        : relative_path(path), volume(vol), pitch(p) {
    }
};

struct PlayMusic {
    std::string relative_path;
    unsigned int volume = 100;
    float pitch = 1.f;
    bool loop = false;

    PlayMusic(std::string_view path, unsigned int vol = 100, float p = 1.f, bool l = false)
        : relative_path(path), volume(vol), pitch(p), loop(l) {
    }
};

struct SetVolume {
    AudioCategory category;
    unsigned int volume;
};

struct SetUiState {
    UiState state;
};

struct SetLevelName {
    LevelName level;
};

struct PlayerSourceAdded {
    InputSource source;
    size_t at_index;
};

struct PlayerSourceRemoved {
    InputSource source;
    size_t at_index;
};

struct ShouldDetectNewPlayerSources {
    bool value;
};

struct ChangeLevelZoom {
    float amount;
    bool smooth;
};

struct SendNotification {
    std::string message;
    std::function<void()> on_click = nullptr;
};

struct GamepadConnectedNotification {
    SDL_JoystickID id;
};

struct SaveUserData {
    UserDataTypes type;
};
}

using GameEvent = std::variant<
    game_event_types::CloseWindow,
    game_event_types::PlaySound,
    game_event_types::PlayMusic,
    game_event_types::SetVolume,
    game_event_types::Input,
    game_event_types::SetUiState,
    game_event_types::SetLevelName,
    game_event_types::PlayerSourceAdded,
    game_event_types::PlayerSourceRemoved,
    game_event_types::ShouldDetectNewPlayerSources,
    game_event_types::ChangeLevelZoom,
    game_event_types::SendNotification,
    game_event_types::GamepadConnectedNotification,
    game_event_types::SaveUserData>;

struct ScheduledEvent {
    Uint64 execute_time_ms;
    GameEvent event;
    bool operator>(const ScheduledEvent& other) const {
        return execute_time_ms > other.execute_time_ms;
    }
};

namespace game_events {
bool poll(GameEvent& event);
void push(GameEvent event);
void schedule(GameEvent event, Uint64 delay_ms);
void update_scheduled_events();
}