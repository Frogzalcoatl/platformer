#include "Gamepad.hpp"
#include <cassert>
#include <cmath>
#include <filesystem>

static int openMappingsFromPath(std::filesystem::path& path) {
    std::string pathString = path.string();
    const char* fullPath = pathString.c_str();
    int mappingsAdded = SDL_AddGamepadMappingsFromFile(fullPath);
    if (mappingsAdded == -1) {
        mappingsAdded = 0;
    }
    SDL_Log("Added %d mapping(s) from %s", mappingsAdded, fullPath);
    return mappingsAdded;
}

bool addGamepadMappingsFromFiles() {
    const char* rawPath = SDL_GetBasePath();
    if (!rawPath) {
        return false;
    }
    std::filesystem::path basePath(rawPath);
    std::filesystem::path communityResourcePath =
        basePath / "assets" / "gamepads" / "gamecontrollerdb.txt";
    std::filesystem::path personalPath =
        basePath / "assets" / "gamepads" / "personalcontrollerdb.txt";
    int mappingsAdded = 0;
    mappingsAdded += openMappingsFromPath(communityResourcePath);
    mappingsAdded += openMappingsFromPath(personalPath);
    return mappingsAdded > 0;
}

static std::array<
    std::array<SDL_GamepadButton, MaxBindsPerVerb>, static_cast<size_t>(InputVerb::Count)>
    buttonBindings = {};

// Sets all values to invalid, representing empty.
void initGamepadButtonBindings() {
    for (int i = 0; i < static_cast<int>(InputVerb::Count); i++) {
        buttonBindings[i].fill(SDL_GAMEPAD_BUTTON_INVALID);
    }
}

void bindGamepadButtonToVerb(
    InputVerb verb, SDL_GamepadButton button, std::optional<int> atIndexOpt
) {
    assert(verb >= static_cast<InputVerb>(0) && verb < InputVerb::Count);
    if (button <= SDL_GAMEPAD_BUTTON_INVALID || button >= SDL_GAMEPAD_BUTTON_COUNT) {
        return;
    }
    auto& bindings = buttonBindings[static_cast<size_t>(verb)];
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

void unbindGamepadButtonFromVerb(InputVerb verb, SDL_GamepadButton button) {
    assert(verb >= static_cast<InputVerb>(0) && verb < InputVerb::Count);
    if (button <= SDL_GAMEPAD_BUTTON_INVALID || button >= SDL_GAMEPAD_BUTTON_COUNT) {
        return;
    }
    auto& bindings = buttonBindings[static_cast<size_t>(verb)];
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (bindings[i] == button) {
            bindings[i] = SDL_GAMEPAD_BUTTON_INVALID; // Invalid represents empty
        }
    }
}

void clearGamepadButtonBindingAtIndex(InputVerb verb, int index) {
    assert(verb >= static_cast<InputVerb>(0) && verb < InputVerb::Count);
    assert(index >= 0 && index < MaxBindsPerVerb);
    auto& bindings = buttonBindings[static_cast<size_t>(verb)];
    bindings[index] = SDL_GAMEPAD_BUTTON_INVALID; // Invalid represents empty
}

std::vector<InputVerb> getVerbsFromGamepadButton(SDL_GamepadButton button) {
    std::vector<InputVerb> verbs = {};
    if (button <= SDL_GAMEPAD_BUTTON_INVALID || button >= SDL_GAMEPAD_BUTTON_COUNT) {
        return verbs;
    }
    for (int i = 0; i < static_cast<int>(InputVerb::Count); i++) {
        for (SDL_GamepadButton& buttonBinding : buttonBindings[i]) {
            if (buttonBinding == button) {
                verbs.push_back(static_cast<InputVerb>(i));
            }
        }
    }
    return verbs;
}

struct DefaultButtonBinding {
    InputVerb verb;
    SDL_GamepadButton button;
};
static const std::vector<DefaultButtonBinding> defaultButtonBindings = {
    {InputVerb::Up, SDL_GAMEPAD_BUTTON_DPAD_UP},
    {InputVerb::Down, SDL_GAMEPAD_BUTTON_DPAD_DOWN},
    {InputVerb::Left, SDL_GAMEPAD_BUTTON_DPAD_LEFT},
    {InputVerb::Right, SDL_GAMEPAD_BUTTON_DPAD_RIGHT},
    // South is A on Xbox, X on PlayStation, B on Switch
    {InputVerb::Confirm, SDL_GAMEPAD_BUTTON_SOUTH},
    // East is B on Xbox, O on PlayStation, A on Switch
    {InputVerb::Cancel, SDL_GAMEPAD_BUTTON_EAST},
    {InputVerb::Cancel, SDL_GAMEPAD_BUTTON_BACK},
    {InputVerb::Jump, SDL_GAMEPAD_BUTTON_SOUTH},
    {InputVerb::Jump, SDL_GAMEPAD_BUTTON_EAST},
};

void bindDefaultGamepadButtons() {
    for (const auto& binding : defaultButtonBindings) {
        bindGamepadButtonToVerb(binding.verb, binding.button, std::nullopt);
    }
}