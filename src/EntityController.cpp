#include "EntityController.hpp"
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

EntityController::EntityController(Entity& entity, std::optional<SDL_JoystickID> joystickId)
    : entity(&entity), joystickId(joystickId) {
}

void EntityController::setEntity(Entity& entity) {
    this->entity = &entity;
}
void EntityController::clearEntity() {
    entity = nullptr;
}
Entity* EntityController::getEntity() const {
    return entity;
}

void EntityController::update() {
    if (!entity || entity->isStatic) {
        return;
    }
    b2BodyId bodyId = entity->getBodyId();
    b2Vec2 velocity = b2Body_GetLinearVelocity(bodyId);
    b2Vec2 targetVelocity = {
        0.f,
        0.f,
    };
    if (movement[static_cast<size_t>(EntityMovement::Down)]) {
        targetVelocity.y -= downwardAcceleration;
    }
    if (movement[static_cast<size_t>(EntityMovement::Left)]) {
        targetVelocity.x -= horizontalSpeed;
    }
    if (movement[static_cast<size_t>(EntityMovement::Right)]) {
        targetVelocity.x += horizontalSpeed;
    }
    if (isSprinting) {
        targetVelocity.x *= sprintMultiplier;
        targetVelocity.y *= sprintMultiplier;
    }
    velocity.x = velocity.x + (targetVelocity.x - velocity.x) * horizontalAcceleration;
    velocity.y += targetVelocity.y;
    b2Body_SetLinearVelocity(bodyId, velocity);
}

void EntityController::jump() {
    if (!entity) {
        return;
    }
    b2BodyId bodyId = entity->getBodyId();
    b2Vec2 velocity = b2Body_GetLinearVelocity(bodyId);
    b2Body_SetLinearVelocity(bodyId, b2Vec2{velocity.x, 0.f});
    b2Body_ApplyLinearImpulseToCenter(bodyId, b2Vec2{0.f, jumpForceNewtons}, true);
}

void EntityController::respawn() {
    if (!entity) {
        return;
    }
    entity->teleport(spawnPoint);
}

void EntityController::handleInput(GameEventTypes::Input event, Camera* camera) {
    assert(event.state == InputState::Pressed || event.state == InputState::Released);
    assert(event.verb < InputVerb::VerbCount);
    if (!entity) {
        return;
    }
    if (event.state == InputState::Pressed) {
        if (event.verb == InputVerb::Respawn) {
            respawn();
            if (camera && camera->entityToFollow == entity) {
                camera->centerOnEntity();
            }
        } else if (event.verb == InputVerb::Jump) {
            float pitch = SDL_randf() * (1.25f - 1.f) + 1.f;
            GameEvents::Push(GameEventTypes::PlaySound{GameAssets::Sounds::Jump, 100, pitch});
            jump();
        }
    }
    if (event.verb == InputVerb::Sprint) {
        isSprinting = event.state == InputState::Pressed;
    }
    auto directionOpt = inputVerbToDirection(event.verb);
    if (!directionOpt.has_value()) {
        return;
    }
    EntityMovement direction = directionOpt.value();
    bool shouldBeMoving = event.state == InputState::Pressed;
    movement[static_cast<size_t>(direction)] = shouldBeMoving;
}

void EntityController::resetMovement() {
    for (size_t i = 0; i < movement.size(); i++) {
        movement[i] = false;
    }
    isSprinting = false;
}