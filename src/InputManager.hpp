#pragma once
#include <SDL3/SDL.h>
#include <array>
#include <filesystem>
#include <optional>
#include <vector>

constexpr int MaxBindsPerVerb = 4;

enum class InputVerb : uint8_t {
    Up,
    Down,
    Left,
    Right,
    Jump,
    Sprint,
    Confirm,
    Cancel,
    Respawn,
    ToggleFullscreen,
    ZoomIn,
    ZoomOut,
    ZoomReset,
    Count
};

std::string inputVerbToString(InputVerb verb);

enum class InputState : uint8_t { Pressed, Released };

struct InputVerbInfo {
    InputVerb verb;
    bool activateOnRepeat = false;
};

struct ScancodeInfo {
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    bool activateOnRepeat = false;
};

struct DefaultScancodeBinding {
    InputVerb verb;
    SDL_Scancode scancode;
    bool activateOnRepeat = false;
};

struct DefaultButtonBinding {
    InputVerb verb;
    SDL_GamepadButton button;
};

class InputManager {
  private:
    std::array<std::array<ScancodeInfo, MaxBindsPerVerb>, static_cast<size_t>(InputVerb::Count)>
        scancodeBindings = {};
    std::array<
        std::array<SDL_GamepadButton, MaxBindsPerVerb>, static_cast<size_t>(InputVerb::Count)>
        gButtonBindings = {};

    int openMappingsFromPath(std::filesystem::path& path);
    bool addGamepadMappingsFromFiles();

  public:
    InputManager();
    void bindScancodeToVerb(InputVerb verb, ScancodeInfo binding, std::optional<int> atIndexOpt);
    void unbindScancodeFromVerb(InputVerb verb, SDL_Scancode scancode);
    void clearScancodeBindingAtIndex(InputVerb verb, int index);
    std::vector<InputVerbInfo> getVerbsFromScancode(SDL_Scancode scancode);
    std::array<ScancodeInfo, MaxBindsPerVerb> getScancodesFromVerb(InputVerb verb);
    void bindGamepadButtonToVerb(
        InputVerb verb, SDL_GamepadButton button, std::optional<int> atIndexOpt
    );
    void unbindGamepadButtonFromVerb(InputVerb verb, SDL_GamepadButton button);
    void clearGamepadButtonBindingAtIndex(InputVerb verb, int index);
    std::vector<InputVerb> getVerbsFromGamepadButton(SDL_GamepadButton button);
    std::array<SDL_GamepadButton, MaxBindsPerVerb> getGamepadButtonsFromverb(InputVerb verb);

    // Will probably remove default bindings vectors later, currently here for convenience
    const std::vector<DefaultScancodeBinding> defaultVerbBindings = {
        {InputVerb::Up, SDL_SCANCODE_W},
        {InputVerb::Up, SDL_SCANCODE_UP},
        {InputVerb::Down, SDL_SCANCODE_S},
        {InputVerb::Down, SDL_SCANCODE_DOWN},
        {InputVerb::Left, SDL_SCANCODE_A},
        {InputVerb::Left, SDL_SCANCODE_LEFT},
        {InputVerb::Right, SDL_SCANCODE_D},
        {InputVerb::Right, SDL_SCANCODE_RIGHT},
        {InputVerb::Confirm, SDL_SCANCODE_RETURN},
        {InputVerb::Cancel, SDL_SCANCODE_ESCAPE},
        {InputVerb::ToggleFullscreen, SDL_SCANCODE_F11},
        {InputVerb::Respawn, SDL_SCANCODE_R},
        {InputVerb::ZoomIn, SDL_SCANCODE_KP_PLUS, true},
        {InputVerb::ZoomOut, SDL_SCANCODE_KP_MINUS, true},
        {InputVerb::ZoomIn, SDL_SCANCODE_EQUALS, true},
        {InputVerb::ZoomOut, SDL_SCANCODE_MINUS, true},
        {InputVerb::ZoomReset, SDL_SCANCODE_0},
        {InputVerb::ZoomReset, SDL_SCANCODE_KP_0},
        {InputVerb::Jump, SDL_SCANCODE_W},
        {InputVerb::Jump, SDL_SCANCODE_UP},
        {InputVerb::Sprint, SDL_SCANCODE_SPACE}
    };
    const std::vector<DefaultButtonBinding> defaultButtonBindings = {
        {InputVerb::Up, SDL_GAMEPAD_BUTTON_DPAD_UP},
        {InputVerb::Down, SDL_GAMEPAD_BUTTON_DPAD_DOWN},
        {InputVerb::Left, SDL_GAMEPAD_BUTTON_DPAD_LEFT},
        {InputVerb::Right, SDL_GAMEPAD_BUTTON_DPAD_RIGHT},
        {InputVerb::Confirm, SDL_GAMEPAD_BUTTON_SOUTH},
        {InputVerb::Cancel, SDL_GAMEPAD_BUTTON_EAST},
        {InputVerb::Cancel, SDL_GAMEPAD_BUTTON_BACK},
        {InputVerb::Jump, SDL_GAMEPAD_BUTTON_SOUTH},
        {InputVerb::Jump, SDL_GAMEPAD_BUTTON_EAST},
        {InputVerb::Sprint, SDL_GAMEPAD_BUTTON_NORTH},
        {InputVerb::Sprint, SDL_GAMEPAD_BUTTON_WEST},
        {InputVerb::Sprint, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER},
        {InputVerb::Sprint, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER}
    };
};
