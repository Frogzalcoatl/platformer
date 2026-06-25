#include "input.hpp"
#include <assert.h>

static std::array<std::array<ScancodeBinding, MaxBindsPerVerb>, InputVerb_Count> verbScancodeBindings = {};

void bindScancodeToVerb(InputVerb verb, ScancodeBinding binding, std::optional<int> atIndexOpt) {
    assert(verb >= 0 && verb < InputVerb_Count);
    if (binding.scancode <= SDL_SCANCODE_UNKNOWN || binding.scancode >= SDL_SCANCODE_COUNT) {
        return;
    }
    // Using a reference instead of a pointer.
    /*
    Differences between reference and pointer:
    - References are not a memory address, rather a direct reference to an existing variable.
    - References are never null since they are attached to existing values.
    - reassigning the below verbBinds variable to something else would also reassign the array above.
    - Cleaner syntax
    */
    auto& verbBinds = verbScancodeBindings[verb];
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (verbBinds[i].scancode == binding.scancode) {
            verbBinds[i].activateOnRepeat = binding.activateOnRepeat;
            return;
        }
    }
    if (atIndexOpt.has_value()) {
        const int atIndex = atIndexOpt.value();
        assert(atIndex >= 0 && atIndex < MaxBindsPerVerb);
        verbBinds[atIndex] = binding;
        return;
    }
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (verbBinds[i].scancode == SDL_SCANCODE_UNKNOWN) {
            verbBinds[i] = binding;
            return;
        }
    }
}

void unbindScancodeFromVerb(InputVerb verb, SDL_Scancode scancode) {
    assert(verb >= 0 && verb < InputVerb_Count);
    assert(scancode > SDL_SCANCODE_UNKNOWN && scancode < SDL_SCANCODE_COUNT);
    auto& verbBinds = verbScancodeBindings[verb];
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (verbBinds[i].scancode == scancode) {
            verbBinds[i].scancode = SDL_SCANCODE_UNKNOWN;
            verbBinds[i].activateOnRepeat = false;
        }
    }
}

// Clears specific index of a verb mapping array
void clearVerbScancodeIndex(InputVerb verb, int index) {
    assert(verb >= 0 && verb < InputVerb_Count);
    assert(index >= 0 && index < MaxBindsPerVerb);
    auto& verbBinds = verbScancodeBindings[verb];
    verbBinds[index].scancode = SDL_SCANCODE_UNKNOWN;
    verbBinds[index].activateOnRepeat = false;
}

std::optional<VerbBinding> getBindingFromScancode(SDL_Scancode scancode) {
    for (int i = 0; i < InputVerb_Count; i++) {
        for (ScancodeBinding binding : verbScancodeBindings[i]) {
            if (binding.scancode == scancode) {
                return VerbBinding{static_cast<InputVerb>(i), binding.activateOnRepeat};
            }
        }
    }
    return std::nullopt;
}

struct FullBinding {
    InputVerb verb;
    SDL_Scancode scancode;
    bool activateOnRepeat = false;
};
static const std::vector<FullBinding> defaultVerbBindings = {{InputVerb_Up, SDL_SCANCODE_W},
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
                                                             {InputVerb_ResetZoom, SDL_SCANCODE_KP_0}};

void connectDefaultVerbMappings(void) {
    for (FullBinding binding : defaultVerbBindings) {
        bindScancodeToVerb(binding.verb, ScancodeBinding{binding.scancode, binding.activateOnRepeat}, std::nullopt);
    }
}