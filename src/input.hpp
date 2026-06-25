#pragma once
#include <SDL3/SDL.h>
#include <array>
#include <optional>
#include <vector>

enum InputVerb {
    InputVerb_Up,
    InputVerb_Down,
    InputVerb_Left,
    InputVerb_Right,
    InputVerb_Confirm,
    InputVerb_Cancel,
    InputVerb_Respawn,
    InputVerb_ToggleFullscreen,
    InputVerb_Count
};

enum InputState { InputState_Pressed, InputState_Released };

struct InputEvent {
    InputVerb verb;
    InputState state;
};

constexpr int MaxBindsPerVerb = 4;

void connectScancodeToVerb(InputVerb verb, SDL_Scancode scancode, std::optional<int> atIndexOpt);
void disconnectScancodeFromVerb(InputVerb verb, SDL_Scancode scancode);
void clearVerbScancodeIndex(InputVerb verb, int index);
std::optional<InputVerb> getVerbFromScancode(SDL_Scancode scancode);
void connectDefaultVerbMappings(void);
