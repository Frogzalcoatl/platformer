#include "playerInput.hpp"
#include "Camera.hpp"
#include "Entity.hpp"
#include "Events.hpp"
#include <cassert>

static std::optional<EntityMovement> inputVerbToDirection(InputVerb verb) {
    switch (verb) {
    case InputVerb::InputVerb_Up:
        return EntityMovement_Up;
    case InputVerb::InputVerb_Down:
        return EntityMovement_Down;
    case InputVerb::InputVerb_Left:
        return EntityMovement_Left;
    case InputVerb::InputVerb_Right:
        return EntityMovement_Right;
    default:
        return std::nullopt;
    }
}

void controlEntity(GameEventTypes::Input event, Entity& entity, Camera& camera) {
    assert(event.state == InputState_Pressed || event.state == InputState_Released);
    assert(event.verb >= 0 && event.verb < InputVerb_Count);
    if (event.state == InputState_Pressed) {
        if (event.verb == InputVerb_Respawn) {
            entity.respawn();
            if (camera.entityToFollow == &entity) {
                camera.centerOnEntity();
            }
            return;
        } else if (event.verb == InputVerb_Jump) {
            entity.jump();
        }
    }
    auto directionOpt = inputVerbToDirection(event.verb);
    if (!directionOpt.has_value()) {
        return;
    }
    EntityMovement direction = directionOpt.value();
    bool shouldBeMoving = event.state == InputState_Pressed;
    entity.movement[direction] = shouldBeMoving;
}