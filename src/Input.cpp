#include "Input.hpp"
#include <cassert>

static std::array<std::array<ScancodeInfo, MaxBindsPerVerb>, InputVerb_Count> scancodeBindings = {};

void bindScancodeToVerb(InputVerb verb, ScancodeInfo scancodeInfo, std::optional<int> atIndexOpt) {
    assert(verb >= 0 && verb < InputVerb_Count);
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
    auto& bindings = scancodeBindings[verb];
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

void unbindScancodeFromVerb(InputVerb verb, SDL_Scancode scancode) {
    assert(verb >= 0 && verb < InputVerb_Count);
    if (scancode <= SDL_SCANCODE_UNKNOWN || scancode >= SDL_SCANCODE_COUNT) {
        return;
    }
    auto& bindings = scancodeBindings[verb];
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (bindings[i].scancode == scancode) {
            bindings[i].scancode = SDL_SCANCODE_UNKNOWN;
            bindings[i].activateOnRepeat = false;
        }
    }
}

// Clears specific index of a verb bind array
void clearScancodeBindingAtIndex(InputVerb verb, int index) {
    assert(verb >= 0 && verb < InputVerb_Count);
    assert(index >= 0 && index < MaxBindsPerVerb);
    auto& bindings = scancodeBindings[verb];
    bindings[index].scancode = SDL_SCANCODE_UNKNOWN;
    bindings[index].activateOnRepeat = false;
}

std::vector<InputVerbInfo> getVerbsFromScancode(SDL_Scancode scancode) {
    std::vector<InputVerbInfo> inputVerbs = {};
    if (scancode <= SDL_SCANCODE_UNKNOWN || scancode >= SDL_SCANCODE_COUNT) {
        return inputVerbs;
    }
    for (int i = 0; i < InputVerb_Count; i++) {
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

struct DefaultScancodeBinding {
    InputVerb verb;
    SDL_Scancode scancode;
    bool activateOnRepeat = false;
};
static const std::vector<DefaultScancodeBinding> defaultVerbBindings = {
    {InputVerb_Up, SDL_SCANCODE_W},
    {InputVerb_Up, SDL_SCANCODE_UP},
    {InputVerb_Down, SDL_SCANCODE_S},
    {InputVerb_Down, SDL_SCANCODE_DOWN},
    {InputVerb_Left, SDL_SCANCODE_A},
    {InputVerb_Left, SDL_SCANCODE_LEFT},
    {InputVerb_Right, SDL_SCANCODE_D},
    {InputVerb_Right, SDL_SCANCODE_RIGHT},
    {InputVerb_Confirm, SDL_SCANCODE_RETURN},
    {InputVerb_Cancel, SDL_SCANCODE_ESCAPE},
    {InputVerb_ToggleFullscreen, SDL_SCANCODE_F11},
    {InputVerb_Respawn, SDL_SCANCODE_R},
    {InputVerb_ZoomIn, SDL_SCANCODE_KP_PLUS, true},
    {InputVerb_ZoomOut, SDL_SCANCODE_KP_MINUS, true},
    {InputVerb_ZoomIn, SDL_SCANCODE_EQUALS, true},
    {InputVerb_ZoomOut, SDL_SCANCODE_MINUS, true},
    {InputVerb_ResetZoom, SDL_SCANCODE_0},
    {InputVerb_ResetZoom, SDL_SCANCODE_KP_0},
    {InputVerb_Jump, SDL_SCANCODE_W},
    {InputVerb_Jump, SDL_SCANCODE_UP},
    {InputVerb_Jump, SDL_SCANCODE_SPACE}
};

void bindDefaultScancodes() {
    for (const DefaultScancodeBinding& binding : defaultVerbBindings) {
        bindScancodeToVerb(
            binding.verb, ScancodeInfo{binding.scancode, binding.activateOnRepeat}, std::nullopt
        );
    }
}