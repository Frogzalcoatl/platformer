#pragma once
#include <SDL3/SDL.h>
#include <optional>
#include <string>
#include <variant>

enum class InputVerb : uint8_t {
    Up,
    Down,
    Left,
    Right,
    Jump,
    Sprint,
    Respawn,
    Confirm,
    Cancel,
    Pause,
    ZoomIn,
    ZoomOut,
    ZoomReset,
    ToggleFullscreen,
    ToggleDebug,
    ShowHitboxes,
    VerbCount
};

struct InputVerbInfo {
    InputVerb verb;
    bool activateOnRepeat = false;
};

enum class InputState : uint8_t {
    Pressed,
    Released,
    InputStateCount
};

enum class InputSource : uint8_t {
    Keyboard,
    Mouse,
    Controller,
    Touch,
    InputSourceCount
};

struct DefaultScancodeBinding {
    InputVerb verb;
    SDL_Scancode scancode;
    bool activateOnRepeat = false;
};

struct DefaultButtonBinding {
    InputVerb verb;
    SDL_GamepadButton button;
};

enum class AudioCategory : uint8_t {
    Master,
    Sounds,
    Music,
    AudioCategoryCount,
};

enum class UiState : uint8_t {
    MainMenu,
    Settings,
    Playing,
    Paused,
    PausedSettings,
    UiStateCount
};

enum class LevelName : uint8_t {
    None,
    Template,
    LevelNameCount
};

namespace GameEventTypes {
struct CloseWindow {};

struct Input {
    InputVerb verb;
    InputState state;
    InputSource source;
    std::optional<SDL_JoystickID> joystickId = std::nullopt;
};

struct PlaySound {
    std::string_view relativePath;
    unsigned int volume = 100;
    float pitch = 1.f;
};

struct PlayMusic {
    std::string_view relativePath;
    unsigned int volume = 100;
    float pitch = 1.f;
    bool loop = false;
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

struct UpdateCurrentPlayers {};
}

using GameEvent = std::variant<
    GameEventTypes::CloseWindow,
    GameEventTypes::PlaySound,
    GameEventTypes::PlayMusic,
    GameEventTypes::SetVolume,
    GameEventTypes::Input,
    GameEventTypes::SetUiState,
    GameEventTypes::SetLevelName,
    GameEventTypes::UpdateCurrentPlayers>;

namespace GameEvents {
bool Poll(GameEvent& event);
void Push(GameEvent event);
}