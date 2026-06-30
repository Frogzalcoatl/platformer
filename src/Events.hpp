#pragma once
#include "InputManager.hpp"
#include <variant>

namespace GameEventTypes {
struct CloseWindow {};
struct PlaySound {
    int soundId;
};
} // namespace GameEventTypes

using GameEvent =
    std::variant<GameEventTypes::CloseWindow, GameEventTypes::PlaySound, GameEventTypes::Input>;

namespace GameEvents {
bool Poll(GameEvent& event);
void Push(GameEvent event);
} // namespace GameEvents