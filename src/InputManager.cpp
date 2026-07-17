#include "InputManager.hpp"
#include "Events.hpp"
#include <algorithm>
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

std::string inputTypeToString(InputType type) {
    switch (type) {
    case InputType::Controller:
        return "Controller";
    case InputType::Keyboard:
        return "Keyboard";
    case InputType::Mouse:
        return "Mouse";
    case InputType::Touch:
        return "Touch";
    default:
        return "Unknown Input Type";
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

const GamepadBindings& InputManager::getGamepadBindings() const {
    return gamepadBindings;
}

const PlayerSources& InputManager::getPlayerSources() const {
    return playerSources;
}

size_t InputManager::getPlayerSourceCount() const {
    return playerSourceCount;
}

int InputManager::sdlGamepadsDetected() const {
    int count = 0;
    SDL_GetGamepads(&count);
    return count;
}

std::string InputManager::getSourceName(const InputSource& source) {
    std::string inputSourceName;
    if (source.type == InputType::Controller) {
        inputSourceName = SDL_GetGamepadNameForID(source.sdlId);
        if (inputSourceName.empty()) {
            inputSourceName = "Controller " + std::to_string(source.sdlId);
        }
    } else {
        inputSourceName = inputTypeToString(source.type);
    }
    return inputSourceName;
}

bool InputManager::addPlayerSource(const InputSource& source) {
    assert(source.type < InputType::InputTypeCount);
    if (playerSourceCount == MaxPlayerSources) {
        return false;
    }
    bool alreadyAdded = std::any_of(
        playerSources.begin(),
        playerSources.end(),
        [&source](const std::optional<InputSource>& playerSource) { return source == playerSource; }
    );
    if (alreadyAdded) {
        return false;
    }
    auto emptyIt = std::find_if(
        playerSources.begin(),
        playerSources.end(),
        [](const std::optional<InputSource>& playerSource) { return !playerSource.has_value(); }
    );
    std::string sourceName = getSourceName(source);
    if (emptyIt == playerSources.end()) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Ignoring attempt to add input source \"%s\": No null index found on playerSources array.",
            sourceName.c_str()
        );
        return false;
    }
    *emptyIt = source;
    playerSourceCount++;
    size_t index = std::distance(playerSources.begin(), emptyIt);
    GameEvents::Push(GameEventTypes::PlayerSourceAdded{source, index});
    SDL_Log("Added player source \"%s\" at index %zu", sourceName.c_str(), index);
    return true;
}

bool InputManager::removePlayerSource(const InputSource& source) {
    auto sourceIt = std::find(playerSources.begin(), playerSources.end(), source);
    std::string sourceName = getSourceName(source);
    if (sourceIt == playerSources.end()) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Ignoring request to remove input source \"%s\": They are not currently on playerSources array.",
            sourceName.c_str()
        );
        return false;
    }
    *sourceIt = std::nullopt;
    if (playerSourceCount == 0) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Ignored attempt decrement an unsigned playerSourceCount of 0. May be out of sync."
        );
    } else {
        playerSourceCount--;
    }
    // Learned about std::stable_partition from AI
    // stable partition shifts all values that return true to the starting indices
    // shifts all values that return false to the indices after.
    std::stable_partition(
        playerSources.begin(), playerSources.end(), [](const std::optional<InputSource>& source) {
            return source.has_value();
        }
    );
    size_t index = std::distance(playerSources.begin(), sourceIt);
    GameEvents::Push(GameEventTypes::PlayerSourceRemoved{source, index});
    SDL_Log("Removed player source \"%s\" at index %zu", sourceName.c_str(), index);
    return true;
}

void InputManager::handleGamepadRemoved(SDL_GamepadDeviceEvent& event) {
    if (event.type != SDL_EVENT_GAMEPAD_REMOVED) {
        return;
    }
    InputSource gamepadSource{InputType::Controller, event.which};
    removePlayerSource(gamepadSource);
}

bool InputManager::hasTouchScreen() {
    int touchDeviceCount;
    SDL_TouchID* touchDevices = SDL_GetTouchDevices(&touchDeviceCount);
    if (touchDeviceCount == 0) {
        return false;
    }
    for (int i = 0; i < touchDeviceCount; i++) {
        SDL_TouchDeviceType deviceType = SDL_GetTouchDeviceType(touchDevices[i]);
        if (deviceType == SDL_TOUCH_DEVICE_DIRECT) {
            return true;
        }
    }
    return false;
}

bool InputManager::enableTouchPlayer() {
    if (!hasTouchScreen()) {
        SDL_Log("Not enabling touch player. No touch screen detected");
        return false;
    }
    return addPlayerSource(DefaultTouchSource);
}

bool InputManager::disableTouchPlayer() {
    return removePlayerSource(DefaultTouchSource);
}

void InputManager::removePlayerSourceAtIndex(size_t index) {
    if (index >= playerSources.size()) {
        SDL_Log("Unable to remove player at invalid index %zu.", index);
        return;
    }
    if (!playerSources[index].has_value()) {
        return;
    }
    // This is technically inefficient, but since theres only 4 players it does not matter.
    // Just so i dont have to worry about keeping the logic for removing players in check at two
    // different places.
    removePlayerSource(playerSources[index].value());
}

std::vector<GameEventTypes::Input> InputManager::handleKeyboardEvent(SDL_KeyboardEvent& event) {
    if (listenForValidKeyboard && event.type == SDL_EVENT_KEY_DOWN) {
        bool addResult = addPlayerSource(DefaultKeyboardSource);
        if (addResult) {
            return {};
        }
    }
    std::vector<GameEventTypes::Input> inputEvents;
    if (event.scancode == SDL_SCANCODE_AC_BACK && event.type == SDL_EVENT_KEY_DOWN) {
        // Always return input pause and cancel events for android back button.
        inputEvents.push_back(
            GameEventTypes::Input{InputVerb::Pause, InputState::Pressed, DefaultTouchSource}
        );
        inputEvents.push_back(
            GameEventTypes::Input{InputVerb::Cancel, InputState::Pressed, DefaultTouchSource}
        );
        return inputEvents;
    }
    std::vector<InputVerbInfo> verbs = getVerbsFromScancode(event.scancode);
    if (verbs.size() == 0) {
        return {};
    }
    for (size_t i = 0; i < verbs.size(); i++) {
        auto& amountPressed = keyboardVerbsPressed[static_cast<size_t>(verbs[i].verb)];
        if (event.type == SDL_EVENT_KEY_DOWN && !event.repeat) {
            amountPressed++;
        } else if (event.type == SDL_EVENT_KEY_UP) {
            if (amountPressed == 0) {
                continue;
            }
            amountPressed--;
        }
        if (amountPressed == 0) {
            inputEvents.push_back(
                GameEventTypes::Input{verbs[i].verb, InputState::Released, DefaultKeyboardSource}
                // Not using event.which bc its somewhat unrekable and who
                // needs to use multiple keyboards at once anyways
            );
        }
        if (event.type == SDL_EVENT_KEY_DOWN && amountPressed >= 1 &&
            (verbs[i].activateOnRepeat || !event.repeat)) {
            inputEvents.push_back(
                GameEventTypes::Input{verbs[i].verb, InputState::Pressed, DefaultKeyboardSource}
            );
        }
    }
    return inputEvents;
}

std::vector<GameEventTypes::Input> InputManager::handleMouseWheelEvent(SDL_MouseWheelEvent& event) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return {};
    }
    if (event.integer_y > 0) {
        // mouse.which is not relevant here
        return {GameEventTypes::Input{InputVerb::ZoomIn, InputState::Pressed, DefaultMouseSource}};
    } else if (event.integer_y < 0) {
        return {GameEventTypes::Input{InputVerb::ZoomOut, InputState::Pressed, DefaultMouseSource}};
    } else {
        return {};
    }
}

std::vector<GameEventTypes::Input>
InputManager::handleGamepadButtonEvent(SDL_GamepadButtonEvent& event) {
    if (listenForNewGamepad && event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        InputSource eventSource{InputType::Controller, event.which};
        bool addResult = addPlayerSource(eventSource);
        if (addResult) {
            return {};
        }
    }
    std::vector<InputVerb> verbs =
        getVerbsFromGamepadButton(static_cast<SDL_GamepadButton>(event.button));
    if (verbs.empty()) {
        return {};
    }
    std::vector<GameEventTypes::Input> inputEvents;
    auto& amountPressedArr = gamepadsVerbsPressed[event.which];
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
                    verbs[i], InputState::Released, InputSource{InputType::Controller, event.which}
                }
            );
        } else if (amountPressed >= 1 && event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
            inputEvents.push_back(
                GameEventTypes::Input{
                    verbs[i], InputState::Pressed, InputSource{InputType::Controller, event.which}
                }
            );
        }
    }
    return inputEvents;
}

std::vector<GameEventTypes::Input> InputManager::getInputEventsFromSDLEvent(SDL_Event& event) {
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        return handleKeyboardEvent(event.key);
    }; break;
    case SDL_EVENT_MOUSE_WHEEL: {
        return handleMouseWheelEvent(event.wheel);
    }; break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        return handleGamepadButtonEvent(event.gbutton);
    }; break;
    default: {
        return {};
    }
    }
}