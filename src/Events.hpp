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

enum class InputType : uint8_t {
    Keyboard,
    Mouse,
    Controller,
    Touch,
    InputTypeCount
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

struct InputSource {
    InputType type;
    Uint32 sdlId;
    // operator == is a suggestion from AI.
    // Means i can simply compare inputSources with ==
    // instead of having to compare the two individual properties.
    bool operator==(const InputSource& other) const = default;
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
    PlayerSourceSetup,
    Playing,
    Paused,
    PausedSettings,
    UiStateCount
};

enum class LevelName : uint8_t {
    None,
    Test,
    LevelNameCount
};

namespace GameEventTypes {
struct CloseWindow {};

struct Input {
    InputVerb verb;
    InputState state;
    InputSource sourceInfo;
};

struct PlaySound {
    std::string relativePath;
    unsigned int volume = 100;
    float pitch = 1.f;

    // Cleanly accept std::string_view
    PlaySound(std::string_view path, unsigned int vol = 100, float p = 1.f)
        : relativePath(path), volume(vol), pitch(p) {
    }
};

struct PlayMusic {
    std::string relativePath;
    unsigned int volume = 100;
    float pitch = 1.f;
    bool loop = false;

    PlayMusic(std::string_view path, unsigned int vol = 100, float p = 1.f, bool l = false)
        : relativePath(path), volume(vol), pitch(p), loop(l) {
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
    size_t atIndex;
};

struct PlayerSourceRemoved {
    InputSource source;
    size_t atIndex;
};

struct ShouldDetectNewPlayerSources {
    bool value;
};
}

// Learned about std::variant from AI. Seems like a reasonable choice here.
using GameEvent = std::variant<
    GameEventTypes::CloseWindow,
    GameEventTypes::PlaySound,
    GameEventTypes::PlayMusic,
    GameEventTypes::SetVolume,
    GameEventTypes::Input,
    GameEventTypes::SetUiState,
    GameEventTypes::SetLevelName,
    GameEventTypes::PlayerSourceAdded,
    GameEventTypes::PlayerSourceRemoved,
    GameEventTypes::ShouldDetectNewPlayerSources>;

namespace GameEvents {
bool Poll(GameEvent& event);
void Push(GameEvent event);
}