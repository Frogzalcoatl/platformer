#pragma once
#include "Events.hpp"
#include "Input.hpp"
#include <SDL3/SDL.h>

bool addGamepadMappings();
std::optional<GameEventTypes::Input> handleGamepadButtonEvent(SDL_GamepadButtonEvent& event);