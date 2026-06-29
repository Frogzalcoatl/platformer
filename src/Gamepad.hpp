#pragma once
#include "Events.hpp"
#include "Input.hpp"
#include <SDL3/SDL.h>

bool addGamepadMappingsFromFiles();
void initGamepadButtonBindings();
void bindGamepadButtonToVerb(
    InputVerb verb, SDL_GamepadButton button, std::optional<int> atIndexOpt
);
void unbindGamepadButtonFromVerb(InputVerb verb, SDL_GamepadButton button);
void clearGamepadButtonBindingAtIndex(InputVerb verb, int index);
std::vector<InputVerb> getVerbsFromGamepadButton(SDL_GamepadButton button);
void bindDefaultGamepadButtons();