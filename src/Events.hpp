#pragma once
#include "AssetManager.hpp"
#include "AudioManager.hpp"
#include "InputManager.hpp"
#include <variant>

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
} // namespace GameEventTypes

using GameEvent = std::variant<
    GameEventTypes::CloseWindow,
    GameEventTypes::PlaySound,
    GameEventTypes::PlayMusic,
    GameEventTypes::SetVolume,
    GameEventTypes::Input>;

namespace GameEvents {
bool Poll(GameEvent& event);
void Push(GameEvent event);
} // namespace GameEvents