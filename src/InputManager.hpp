#pragma once
#include "Events.hpp"
#include <SDL3/SDL.h>
#include <array>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <vector>

inline constexpr int MaxBindsPerVerb = 3;

std::string inputVerbToString(InputVerb verb);
std::string inputTypeToString(InputType type);

struct ScancodeInfo {
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    bool activateOnRepeat = false;
};

using ScancodeBindings = std::
    array<std::array<ScancodeInfo, MaxBindsPerVerb>, static_cast<size_t>(InputVerb::VerbCount)>;

using GamepadBindings = std::array<
    std::array<SDL_GamepadButton, MaxBindsPerVerb>,
    static_cast<size_t>(InputVerb::VerbCount)>;

inline constexpr size_t MaxPlayerSources = 4;
using PlayerSources = std::array<std::optional<InputSource>, MaxPlayerSources>;

class InputManager {
  private:
    ScancodeBindings scancodeBindings = {};
    GamepadBindings gamepadBindings = {};
    // Arrays of the amount a verb is pressed
    // e.g. two buttons bound to the same verb:
    // Prevents release event when one of the buttons is released and the other isnt.
    std::unordered_map<SDL_JoystickID, std::array<int, static_cast<size_t>(InputVerb::VerbCount)>>
        gamepadsVerbsPressed;

    std::array<int, static_cast<size_t>(InputVerb::VerbCount)> keyboardVerbsPressed = {};

    PlayerSources playerSources = {};
    size_t playerSourceCount = 0;

    const InputSource DefaultTouchSource = InputSource{InputType::Touch, 0};
    const InputSource DefaultKeyboardSource = InputSource{InputType::Keyboard, 0};
    const InputSource DefaultMouseSource = InputSource{InputType::Mouse, 0};

    // Returns true if player source is added/removed
    bool addPlayerSource(const InputSource& source);
    bool removePlayerSource(const InputSource& source);

    std::vector<GameEventTypes::Input> handleKeyboardEvent(SDL_KeyboardEvent& event);
    std::vector<GameEventTypes::Input> handleMouseWheelEvent(SDL_MouseWheelEvent& event);
    std::vector<GameEventTypes::Input> handleGamepadButtonEvent(SDL_GamepadButtonEvent& event);

    bool hasTouchScreen();

  public:
    InputManager();

    void bindScancodeToVerb(InputVerb verb, ScancodeInfo binding, std::optional<int> atIndexOpt);
    void unbindScancodeFromVerb(InputVerb verb, SDL_Scancode scancode);
    void clearScancodeBindingAtIndex(InputVerb verb, int index);
    std::vector<InputVerbInfo> getVerbsFromScancode(SDL_Scancode scancode);
    const std::array<ScancodeInfo, MaxBindsPerVerb>& getScancodesFromVerb(InputVerb verb) const;
    const ScancodeBindings& getScancodeBindings() const;

    void bindGamepadButtonToVerb(
        InputVerb verb, SDL_GamepadButton button, std::optional<int> atIndexOpt
    );
    void unbindGamepadButtonFromVerb(InputVerb verb, SDL_GamepadButton button);
    void clearGamepadButtonBindingAtIndex(InputVerb verb, int index);
    std::vector<InputVerb> getVerbsFromGamepadButton(SDL_GamepadButton button);
    const std::array<SDL_GamepadButton, MaxBindsPerVerb>&
    getGamepadButtonsFromVerb(InputVerb verb) const;
    const GamepadBindings& getGamepadBindings() const;

    const PlayerSources& getPlayerSources() const;

    size_t getPlayerSourceCount() const;

    int sdlGamepadsDetected() const;

    std::string getSourceName(const InputSource& source);

    bool listenForNewGamepad = false;
    bool listenForValidKeyboard = false;

    void handleGamepadRemoved(SDL_GamepadDeviceEvent& event);

    // returns true if touch player is enabled/disabled.
    // only one touch player allowed on device at a time.
    bool enableTouchPlayer();
    bool disableTouchPlayer();

    void removePlayerSourceAtIndex(size_t index);

    bool isTouchPlayerEnabled(size_t* atIndex);

    std::vector<GameEventTypes::Input> getInputEventsFromSDLEvent(SDL_Event& event);

    // void listenForScancodeBinding(InputVerb forVerb, size_t atIndex);

    // void listenForGamepadBinding(InputVerb forVerb, size_t atIndex);

    void handlePinchEvent(SDL_PinchFingerEvent& event);

    // Will probably remove default bindings vectors later, currently here for convenience
    const std::vector<DefaultScancodeBinding> defaultVerbBindings = {
        {InputVerb::Up, SDL_SCANCODE_UP},
        {InputVerb::Down, SDL_SCANCODE_DOWN},
        {InputVerb::Left, SDL_SCANCODE_LEFT},
        {InputVerb::Right, SDL_SCANCODE_RIGHT},
        {InputVerb::Up, SDL_SCANCODE_W},
        {InputVerb::Down, SDL_SCANCODE_S},
        {InputVerb::Left, SDL_SCANCODE_A},
        {InputVerb::Right, SDL_SCANCODE_D},
        {InputVerb::Jump, SDL_SCANCODE_UP},
        {InputVerb::Jump, SDL_SCANCODE_W},
        {InputVerb::Jump, SDL_SCANCODE_SPACE},
        {InputVerb::Sprint, SDL_SCANCODE_LSHIFT},
        {InputVerb::Sprint, SDL_SCANCODE_RSHIFT},
        {InputVerb::Confirm, SDL_SCANCODE_RETURN},
        {InputVerb::Cancel, SDL_SCANCODE_ESCAPE},
        {InputVerb::Pause, SDL_SCANCODE_ESCAPE},
        {InputVerb::ToggleFullscreen, SDL_SCANCODE_F11},
        {InputVerb::Respawn, SDL_SCANCODE_R},
        {InputVerb::ZoomIn, SDL_SCANCODE_KP_PLUS, true},
        {InputVerb::ZoomOut, SDL_SCANCODE_KP_MINUS, true},
        {InputVerb::ZoomIn, SDL_SCANCODE_EQUALS, true},
        {InputVerb::ZoomOut, SDL_SCANCODE_MINUS, true},
        {InputVerb::ZoomReset, SDL_SCANCODE_KP_0},
        {InputVerb::ZoomReset, SDL_SCANCODE_0},
        {InputVerb::ToggleDebug, SDL_SCANCODE_F3},
        {InputVerb::ShowHitboxes, SDL_SCANCODE_F1}
    };
    const std::vector<DefaultButtonBinding> defaultGamepadBindings = {
        {InputVerb::Up, SDL_GAMEPAD_BUTTON_DPAD_UP},
        {InputVerb::Down, SDL_GAMEPAD_BUTTON_DPAD_DOWN},
        {InputVerb::Left, SDL_GAMEPAD_BUTTON_DPAD_LEFT},
        {InputVerb::Right, SDL_GAMEPAD_BUTTON_DPAD_RIGHT},
        {InputVerb::Jump, SDL_GAMEPAD_BUTTON_EAST},
        {InputVerb::Jump, SDL_GAMEPAD_BUTTON_SOUTH},
        {InputVerb::Confirm, SDL_GAMEPAD_BUTTON_EAST},
        {InputVerb::Pause, SDL_GAMEPAD_BUTTON_START},
        {InputVerb::Pause, SDL_GAMEPAD_BUTTON_GUIDE},
        {InputVerb::Cancel, SDL_GAMEPAD_BUTTON_SOUTH},
        {InputVerb::Sprint, SDL_GAMEPAD_BUTTON_WEST},
        {InputVerb::Sprint, SDL_GAMEPAD_BUTTON_NORTH},
    };
};