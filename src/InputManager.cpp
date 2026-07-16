#include "InputManager.hpp"
#include "Events.hpp"
#include <cassert>
#include <imgui.h>

std::string inputVerbToString(InputVerb verb) {
    switch (verb) {
    case InputVerb::Up:
        return "Up";
    case InputVerb::Down:
        return "Down";
    case InputVerb::Left:
        return "Left";
    case InputVerb::Right:
        return "Right";
    case InputVerb::Jump:
        return "Jump";
    case InputVerb::Sprint:
        return "Sprint";
    case InputVerb::Respawn:
        return "Respawn";
    case InputVerb::Confirm:
        return "Confirm";
    case InputVerb::Cancel:
        return "Cancel";
    case InputVerb::Pause:
        return "Pause Game";
    case InputVerb::ZoomIn:
        return "Zoom In";
    case InputVerb::ZoomOut:
        return "Zoom Out";
    case InputVerb::ZoomReset:
        return "Zoom Reset";
    case InputVerb::ToggleFullscreen:
        return "Toggle Fullscreen";
    case InputVerb::ToggleDebug:
        return "Toggle Debug";
    case InputVerb::ShowHitboxes:
        return "Show Hitboxes";
    default:
        return "";
    }
}

InputManager::InputManager() {
    for (int i = 0; i < static_cast<int>(InputVerb::VerbCount); i++) {
        gamepadBindings[i].fill(SDL_GAMEPAD_BUTTON_INVALID); // Invalid represents empty.
    }
    for (const auto& binding : defaultVerbBindings) {
        bindScancodeToVerb(
            binding.verb, ScancodeInfo{binding.scancode, binding.activateOnRepeat}, std::nullopt
        );
    }
    for (const auto& binding : defaultGamepadBindings) {
        bindGamepadButtonToVerb(binding.verb, binding.button, std::nullopt);
    }
}

void InputManager::bindScancodeToVerb(
    InputVerb verb, ScancodeInfo scancodeInfo, std::optional<int> atIndexOpt
) {
    assert(verb < InputVerb::VerbCount);
    if (scancodeInfo.scancode <= SDL_SCANCODE_UNKNOWN ||
        scancodeInfo.scancode >= SDL_SCANCODE_COUNT) {
        return;
    }
    // Using a reference instead of a pointer.
    /*
    Differences between reference and pointer:
    - References are not a memory address, rather a direct reference to an existing variable.
    - References are never null since they are attached to existing values.
    - reassigning the below verbBinds variable to something else would also overwrite the array
    above.
    - Cleaner syntax
    */
    auto& bindings = scancodeBindings[static_cast<size_t>(verb)];
    if (atIndexOpt.has_value()) {
        const int& atIndex = atIndexOpt.value();
        assert(atIndex >= 0 && atIndex < MaxBindsPerVerb);
        bindings[atIndex] = scancodeInfo;
        return;
    }
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (bindings[i].scancode == scancodeInfo.scancode) {
            bindings[i].activateOnRepeat = scancodeInfo.activateOnRepeat;
            return;
        }
    }
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (bindings[i].scancode == SDL_SCANCODE_UNKNOWN) {
            bindings[i] = scancodeInfo;
            return;
        }
    }
}

void InputManager::unbindScancodeFromVerb(InputVerb verb, SDL_Scancode scancode) {
    assert(verb < InputVerb::VerbCount);
    if (scancode <= SDL_SCANCODE_UNKNOWN || scancode >= SDL_SCANCODE_COUNT) {
        return;
    }
    auto& bindings = scancodeBindings[static_cast<size_t>(verb)];
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (bindings[i].scancode == scancode) {
            bindings[i].scancode = SDL_SCANCODE_UNKNOWN;
            bindings[i].activateOnRepeat = false;
        }
    }
}

void InputManager::clearScancodeBindingAtIndex(InputVerb verb, int index) {
    assert(verb < InputVerb::VerbCount);
    assert(index >= 0 && index < MaxBindsPerVerb);
    auto& bindings = scancodeBindings[static_cast<size_t>(verb)];
    bindings[index].scancode = SDL_SCANCODE_UNKNOWN;
    bindings[index].activateOnRepeat = false;
}

std::vector<InputVerbInfo> InputManager::getVerbsFromScancode(SDL_Scancode scancode) {
    std::vector<InputVerbInfo> inputVerbs = {};
    if (scancode <= SDL_SCANCODE_UNKNOWN || scancode >= SDL_SCANCODE_COUNT) {
        return inputVerbs;
    }
    for (int i = 0; i < static_cast<int>(InputVerb::VerbCount); i++) {
        for (ScancodeInfo& binding : scancodeBindings[i]) {
            if (binding.scancode == scancode) {
                inputVerbs.push_back(
                    InputVerbInfo{static_cast<InputVerb>(i), binding.activateOnRepeat}
                );
            }
        }
    }
    return inputVerbs;
}

const std::array<ScancodeInfo, MaxBindsPerVerb>&
InputManager::getScancodesFromVerb(InputVerb verb) const {
    assert(verb < InputVerb::VerbCount);
    return scancodeBindings[static_cast<size_t>(verb)];
}

const ScancodeBindings& InputManager::getScancodeBindings() const {
    return scancodeBindings;
}

void InputManager::bindGamepadButtonToVerb(
    InputVerb verb, SDL_GamepadButton button, std::optional<int> atIndexOpt
) {
    assert(verb < InputVerb::VerbCount);
    if (button <= SDL_GAMEPAD_BUTTON_INVALID || button >= SDL_GAMEPAD_BUTTON_COUNT) {
        return;
    }
    auto& bindings = gamepadBindings[static_cast<size_t>(verb)];
    if (atIndexOpt.has_value()) {
        const int& atIndex = atIndexOpt.value();
        assert(atIndex >= 0 && atIndex < MaxBindsPerVerb);
        bindings[atIndex] = button;
    }
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (bindings[i] == button) {
            return;
        }
    }
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (bindings[i] == SDL_GAMEPAD_BUTTON_INVALID) {
            bindings[i] = button;
            return;
        }
    }
}

void InputManager::unbindGamepadButtonFromVerb(InputVerb verb, SDL_GamepadButton button) {
    assert(verb < InputVerb::VerbCount);
    if (button <= SDL_GAMEPAD_BUTTON_INVALID || button >= SDL_GAMEPAD_BUTTON_COUNT) {
        return;
    }
    auto& bindings = gamepadBindings[static_cast<size_t>(verb)];
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (bindings[i] == button) {
            bindings[i] = SDL_GAMEPAD_BUTTON_INVALID; // Invalid represents empty
        }
    }
}

void InputManager::clearGamepadButtonBindingAtIndex(InputVerb verb, int index) {
    assert(verb < InputVerb::VerbCount);
    assert(index >= 0 && index < MaxBindsPerVerb);
    auto& bindings = gamepadBindings[static_cast<size_t>(verb)];
    bindings[index] = SDL_GAMEPAD_BUTTON_INVALID; // Invalid represents empty
}

std::vector<InputVerb> InputManager::getVerbsFromGamepadButton(SDL_GamepadButton button) {
    std::vector<InputVerb> verbs = {};
    if (button <= SDL_GAMEPAD_BUTTON_INVALID || button >= SDL_GAMEPAD_BUTTON_COUNT) {
        return verbs;
    }
    for (int i = 0; i < static_cast<int>(InputVerb::VerbCount); i++) {
        for (SDL_GamepadButton& buttonBinding : gamepadBindings[i]) {
            if (buttonBinding == button) {
                verbs.push_back(static_cast<InputVerb>(i));
            }
        }
    }
    return verbs;
}

const std::array<SDL_GamepadButton, MaxBindsPerVerb>&
InputManager::getGamepadButtonsFromVerb(InputVerb verb) const {
    assert(verb < InputVerb::VerbCount);
    return gamepadBindings[static_cast<size_t>(verb)];
}

void InputManager::handleGamepadDeviceEvent(SDL_GamepadDeviceEvent& event) {
    if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
        gamepadsVerbsPressed.erase(event.which);
        for (auto it = playerSources.begin(); it != playerSources.end(); it++) {
            if (it->type == InputSource::Controller && it->sdlId == event.which) {
                playerSources.erase(it);
                break;
            }
        }
    } else if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
        gamepadsVerbsPressed[event.which] = {};
        playerSources.push_back(PlayerSourceInfo{InputSource::Controller, event.which});
    }
}

const GamepadBindings& InputManager::getGamepadBindings() const {
    return gamepadBindings;
}

size_t InputManager::getGamepadCount() const {
    return gamepadsVerbsPressed.size();
}

const std::vector<PlayerSourceInfo>& InputManager::getPlayerSources() const {
    return playerSources;
}

std::vector<GameEventTypes::Input> InputManager::getInputEventsFromSDLEvent(SDL_Event& event) {
    std::vector<GameEventTypes::Input> inputEvents;
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        /* Not needed atm
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard != isImGuiCapturingKeyboard) {
            isImGuiCapturingKeyboard = io.WantCaptureKeyboard;
            if (isImGuiCapturingKeyboard) {
                for (size_t i = 0; i < keyboardVerbsPressed.size(); i++) {
                    int& amountPressed = keyboardVerbsPressed[i];
                    if (amountPressed > 0) {
                        amountPressed = 0;
                        inputEvents.push_back(
                            GameEventTypes::Input{
                                static_cast<InputVerb>(i),
                                InputState::Released,
                                InputSource::Keyboard
                            }
                        );
                    }
                }
            }
        }
        if (isImGuiCapturingKeyboard) {
            break;
        }
        */
        if (event.key.scancode == SDL_SCANCODE_AC_BACK && event.key.type == SDL_EVENT_KEY_DOWN) {
            // Always return input pause and cancel events for android back button.
            inputEvents.push_back(
                GameEventTypes::Input{
                    InputVerb::Pause, InputState::Pressed, PlayerSourceInfo{InputSource::Touch, 0}
                }
            );
            inputEvents.push_back(
                GameEventTypes::Input{
                    InputVerb::Cancel, InputState::Pressed, PlayerSourceInfo{InputSource::Touch, 0}
                }
            );
            return inputEvents;
        }
        std::vector<InputVerbInfo> verbs = getVerbsFromScancode(event.key.scancode);
        if (verbs.size() == 0) {
            break;
        }
        for (size_t i = 0; i < verbs.size(); i++) {
            auto& amountPressed = keyboardVerbsPressed[static_cast<size_t>(verbs[i].verb)];
            if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                amountPressed++;
            } else if (event.type == SDL_EVENT_KEY_UP) {
                if (amountPressed == 0) {
                    continue;
                }
                amountPressed--;
            }
            if (amountPressed == 0) {
                // Not using event.which bc its somewhat unreliable and who needs to use multiple
                // keyboards at once
                inputEvents.push_back(
                    GameEventTypes::Input{
                        verbs[i].verb,
                        InputState::Released,
                        PlayerSourceInfo{InputSource::Keyboard, 0}
                    }
                );
            }
            if (amountPressed >= 1 && (verbs[i].activateOnRepeat || !event.key.repeat) &&
                event.type == SDL_EVENT_KEY_DOWN) {
                inputEvents.push_back(
                    GameEventTypes::Input{
                        verbs[i].verb,
                        InputState::Pressed,
                        PlayerSourceInfo{InputSource::Keyboard, 0}
                    }
                );
            }
        }
    }; break;
    case SDL_EVENT_MOUSE_WHEEL: {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            break;
        }
        if (event.wheel.integer_y > 0) {
            // mouse.which is not relevant here
            inputEvents.push_back(
                GameEventTypes::Input{
                    InputVerb::ZoomIn, InputState::Pressed, PlayerSourceInfo{InputSource::Mouse, 0}
                }
            );
        } else if (event.wheel.integer_y < 0) {
            inputEvents.push_back(
                GameEventTypes::Input{
                    InputVerb::ZoomOut, InputState::Pressed, PlayerSourceInfo{InputSource::Mouse, 0}
                }
            );
        }
    }; break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        std::vector<InputVerb> verbs =
            getVerbsFromGamepadButton(static_cast<SDL_GamepadButton>(event.gbutton.button));
        if (verbs.size() == 0) {
            break;
        }
        auto& amountPressedArr = gamepadsVerbsPressed[event.gbutton.which];
        for (size_t i = 0; i < verbs.size(); i++) {
            auto& amountPressed = amountPressedArr[static_cast<size_t>(verbs[i])];
            if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                amountPressed++;
            } else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
                amountPressed--;
            }
            if (amountPressed == 0) {
                inputEvents.push_back(
                    GameEventTypes::Input{
                        verbs[i],
                        InputState::Released,
                        PlayerSourceInfo{InputSource::Controller, event.gbutton.which}
                    }
                );
            } else if (amountPressed >= 1 && event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                inputEvents.push_back(
                    GameEventTypes::Input{
                        verbs[i],
                        InputState::Pressed,
                        PlayerSourceInfo{InputSource::Controller, event.gbutton.which}
                    }
                );
            }
        }
    }; break;
    }
    return inputEvents;
}