#pragma once
#include <SDL3/SDL.h>
#include <array>
#include <optional>
#include <vector>

enum InputVerb : uint8_t {
    InputVerb_Up,
    InputVerb_Down,
    InputVerb_Left,
    InputVerb_Right,
    InputVerb_Confirm,
    InputVerb_Cancel,
    InputVerb_Respawn,
    InputVerb_ToggleFullscreen,
    InputVerb_ZoomIn,
    InputVerb_ZoomOut,
    InputVerb_ResetZoom,
    InputVerb_Jump,
    InputVerb_Count
};

enum InputState : uint8_t { InputState_Pressed, InputState_Released };

struct InputVerbInfo {
    InputVerb verb;
    bool activateOnRepeat = false;
};

struct ScancodeInfo {
    SDL_Scancode scancode;
    bool activateOnRepeat = false;
};

constexpr int MaxBindsPerVerb = 4;

void bindScancodeToVerb(InputVerb verb, ScancodeInfo binding, std::optional<int> atIndexOpt);
void unbindScancodeFromVerb(InputVerb verb, SDL_Scancode scancode);
void clearScancodeBindingAtIndex(InputVerb verb, int index);
std::vector<InputVerbInfo> getVerbsFromScancode(SDL_Scancode scancode);
void bindDefaultScancodes();
