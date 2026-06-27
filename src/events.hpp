#pragma once
#include "input.hpp"
#include <variant>

namespace GameEventTypes {
struct CloseWindow {};
struct PlaySound {
    int soundId;
};
struct ToggleFullscreen {};
struct Input {
    InputVerb verb;
    InputState state;
};
} // namespace GameEventTypes

using GameEvent = std::variant<
    GameEventTypes::CloseWindow, GameEventTypes::PlaySound, GameEventTypes::ToggleFullscreen,
    GameEventTypes::Input>;

namespace GameEvents {
bool Poll(GameEvent& event);
void Push(GameEvent event);
} // namespace GameEvents