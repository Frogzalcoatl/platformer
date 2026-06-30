#include "playerInput.hpp"
#include "Camera.hpp"
#include "Entity.hpp"
#include "Events.hpp"
#include <cassert>

static std::optional<EntityMovement> inputVerbToDirection(InputVerb verb) {
    switch (verb) {
    case InputVerb::Up:
        return EntityMovement::Up;
    case InputVerb::Down:
        return EntityMovement::Down;
    case InputVerb::Left:
        return EntityMovement::Left;
    case InputVerb::Right:
        return EntityMovement::Right;
    default:
        return std::nullopt;
    }
}

void controlEntity(GameEventTypes::Input event, Entity& entity, Camera& camera) {
    assert(event.state == InputState::Pressed || event.state == InputState::Released);
    assert(event.verb >= static_cast<InputVerb>(0) && event.verb < InputVerb::VerbCount);
    if (event.state == InputState::Pressed) {
        if (event.verb == InputVerb::Respawn) {
            entity.respawn();
            if (camera.entityToFollow == &entity) {
                camera.centerOnEntity();
            }
        } else if (event.verb == InputVerb::Jump) {
            float pitch = SDL_randf() * (1.25f - 0.75f) + 0.75f;
            GameEvents::Push(GameEventTypes::PlaySound{GameAssets::Sounds::Fart, 50, pitch});
            entity.jump();
        }
    }
    if (event.verb == InputVerb::Sprint) {
        entity.isSprinting = event.state == InputState::Pressed;
    }
    auto directionOpt = inputVerbToDirection(event.verb);
    if (!directionOpt.has_value()) {
        return;
    }
    EntityMovement direction = directionOpt.value();
    bool shouldBeMoving = event.state == InputState::Pressed;
    entity.movement[static_cast<size_t>(direction)] = shouldBeMoving;
}