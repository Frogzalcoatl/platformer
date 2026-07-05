#pragma once
#include "AssetManager.hpp"
#include "AudioManager.hpp"
#include "InputManager.hpp"
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
    GameAssets::Sounds soundId;
    unsigned int volume = 100;
    float pitch = 1.f;
};
struct PlayMusic {
    GameAssets::Music musicId;
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
} // namespace GameEventTypes

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
} // namespace GameEvents