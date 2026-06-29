#pragma once
#include <SDL3/SDL.h>
#include <array>
#include <optional>
#include <vector>

enum class InputVerb : uint8_t {
    Up,
    Down,
    Left,
    Right,
    Confirm,
    Cancel,
    Respawn,
    ToggleFullscreen,
    ZoomIn,
    ZoomOut,
    ZoomReset,
    Jump,
    Count
};

enum class InputState : uint8_t { Pressed, Released };

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
