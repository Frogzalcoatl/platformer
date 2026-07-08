#pragma once
#include "AssetManager.hpp"
#include "AudioManager.hpp"
#include "InputManager.hpp"
#include <string>
#include <variant>

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