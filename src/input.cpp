#include "input.hpp"
#include <assert.h>

static std::array<std::array<SDL_Scancode, MaxBindsPerVerb>, InputVerb_Count> verbMappings = {};

void connectScancodeToVerb(InputVerb verb, SDL_Scancode scancode, std::optional<int> atIndexOpt) {
    assert(verb >= 0 && verb < InputVerb_Count);
    if (scancode <= SDL_SCANCODE_UNKNOWN || scancode >= SDL_SCANCODE_COUNT) {
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
    auto& verbBinds = verbMappings[verb];
    for (SDL_Scancode boundScancode : verbBinds) {
        if (boundScancode == scancode) {
            return;
        }
    }
    if (atIndexOpt.has_value()) {
        const int atIndex = atIndexOpt.value();
        assert(atIndex >= 0 && atIndex < MaxBindsPerVerb);
        verbBinds[atIndex] = scancode;
        return;
    }
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (verbBinds[i] == SDL_SCANCODE_UNKNOWN) {
            verbBinds[i] = scancode;
            return;
        }
    }
}

void disconnectScancodeFromVerb(InputVerb verb, SDL_Scancode scancode) {
    assert(verb >= 0 && verb < InputVerb_Count);
    assert(scancode > SDL_SCANCODE_UNKNOWN && scancode < SDL_SCANCODE_COUNT);
    auto& verbBinds = verbMappings[verb];
    for (int i = 0; i < MaxBindsPerVerb; i++) {
        if (verbBinds[i] == scancode) {
            verbBinds[i] = SDL_SCANCODE_UNKNOWN;
        }
    }
}

// Clears specific index of a verb mapping array
void clearVerbScancodeIndex(InputVerb verb, int index) {
    assert(verb >= 0 && verb < InputVerb_Count);
    assert(index >= 0 && index < MaxBindsPerVerb);
    auto& verbBinds = verbMappings[verb];
    verbBinds[index] = SDL_SCANCODE_UNKNOWN;
}

std::optional<InputVerb> getVerbFromScancode(SDL_Scancode scancode) {
    for (int i = 0; i < InputVerb_Count; i++) {
        for (SDL_Scancode boundScancode : verbMappings[i]) {
            if (boundScancode == scancode) {
                return static_cast<InputVerb>(i);
            }
        }
    }
    return std::nullopt;
}

struct DefaultBinding {
    InputVerb verb;
    SDL_Scancode scancode;
};
static const std::vector<DefaultBinding> defaultVerbMappings = {{InputVerb_Up, SDL_SCANCODE_W},
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
                                                                {InputVerb_Respawn, SDL_SCANCODE_R}};

void connectDefaultVerbMappings(void) {
    for (DefaultBinding binding : defaultVerbMappings) {
        connectScancodeToVerb(binding.verb, binding.scancode, std::nullopt);
    }
}