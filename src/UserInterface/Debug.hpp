#pragma once
#include "Camera.hpp"
#include "Entity.hpp"
#include "EntityController.hpp"
#include "InputManager.hpp"
#include "WindowManager.hpp"

namespace UserInterface {
void debug(
    WindowManager& window,
    Entity* player,
    EntityController& entityController,
    Camera& camera,
    InputManager& input,
    bool& showFanTriangulation
);
}