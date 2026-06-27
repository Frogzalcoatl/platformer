#include "Gamepad.hpp"
#include <cmath>
#include <filesystem>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

bool addGamepadMappings() {
    const char* rawPath = SDL_GetBasePath();
    if (!rawPath) {
        return false;
    }
    std::filesystem::path basePath(rawPath);
    std::filesystem::path communityResourcePath = basePath / "assets" / "gamecontrollerdb.txt";
    std::filesystem::path personalPath = basePath / "assets" / "personalcontrollerdb.txt";
    int mappingsAdded = 0;
    mappingsAdded += openMappingsFromPath(communityResourcePath);
    mappingsAdded += openMappingsFromPath(personalPath);
    return mappingsAdded > 0;
}

static std::optional<InputVerb> mapButtonToVerb(Uint8 button) {
    switch (button) {
    case SDL_GAMEPAD_BUTTON_DPAD_UP:
        return InputVerb_Up;
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
        return InputVerb_Down;
    case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
        return InputVerb_Left;
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
        return InputVerb_Right;

    // South is A on Xbox, X on PlayStation, B on Switch
    case SDL_GAMEPAD_BUTTON_SOUTH:
        return InputVerb_Confirm;

    // East is B on Xbox, O on PlayStation, A on Switch
    case SDL_GAMEPAD_BUTTON_EAST:
        return InputVerb_Cancel;

    case SDL_GAMEPAD_BUTTON_START:
        return InputVerb_Confirm;
    case SDL_GAMEPAD_BUTTON_BACK:
        return InputVerb_Cancel;
    default:
        return std::nullopt;
    }
}

std::optional<GameEventTypes::Input> handleGamepadButtonEvent(SDL_GamepadButtonEvent& event) {
    auto verbResult = mapButtonToVerb(event.button);
    if (!verbResult.has_value()) {
        return std::nullopt;
    }
    GameEventTypes::Input inputEvent;
    inputEvent.verb = verbResult.value();
    inputEvent.state =
        event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ? InputState_Pressed : InputState_Released;
    return inputEvent;
}