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
    InputVerb_ZoomIn,
    InputVerb_ZoomOut,
    InputVerb_ResetZoom,
    InputVerb_Count
};

enum InputState { InputState_Pressed, InputState_Released };

struct VerbBinding {
    InputVerb verb;
    bool activateOnRepeat = false;
};

struct ScancodeBinding {
    SDL_Scancode scancode;
    bool activateOnRepeat = false;
};

struct InputEvent {
    InputVerb verb;
    InputState state;
};

constexpr int MaxBindsPerVerb = 4;

void bindScancodeToVerb(InputVerb verb, ScancodeBinding binding, std::optional<int> atIndexOpt);
void unbindScancodeFromVerb(InputVerb verb, SDL_Scancode scancode);
void clearVerbScancodeIndex(InputVerb verb, int index);
std::optional<VerbBinding> getBindingFromScancode(SDL_Scancode scancode);
void connectDefaultVerbMappings(void);
