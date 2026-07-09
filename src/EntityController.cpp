#include "EntityController.hpp"
#include "AssetPaths.hpp"
#include "Camera.hpp"
#include "Drawing.hpp"
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

EntityController::EntityController(
    Entity& entity, AssetManager& assets, std::optional<SDL_JoystickID> joystickId
)
    : entity(&entity), joystickId(joystickId) {
    std::string nametagStr = "Player ";
    nametagStr += std::to_string(joystickId.value_or(1));
    nametag = assets.getSDLText(nametagStr, AssetPaths::Fonts::Consolas, 20.f);
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
    if (!entity) {
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

void EntityController::resetMovement() {
    for (size_t i = 0; i < movement.size(); i++) {
        movement[i] = false;
    }
    isSprinting = false;
}

void EntityController::handleInput(GameEventTypes::Input event, Camera* camera, float alpha) {
    assert(event.state == InputState::Pressed || event.state == InputState::Released);
    assert(event.verb < InputVerb::VerbCount);
    if (!entity) {
        return;
    }
    if (event.state == InputState::Pressed) {
        if (event.verb == InputVerb::Respawn) {
            respawn();
            if (camera && camera->entityToFollow == entity) {
                camera->centerOnEntity(alpha);
            }
        } else if (event.verb == InputVerb::Jump) {
            float pitch = SDL_randf() * (1.25f - 1.f) + 1.f;
            GameEvents::Push(GameEventTypes::PlaySound{AssetPaths::Sounds::Jump, 100, pitch});
            jump();
        }
    }
    if (event.verb == InputVerb::Sprint) {
        isSprinting = event.state == InputState::Pressed;
    }
    std::optional<EntityMovement> directionOpt = inputVerbToDirection(event.verb);
    if (!directionOpt.has_value()) {
        return;
    }
    EntityMovement direction = directionOpt.value();
    bool shouldBeMoving = event.state == InputState::Pressed;
    movement[static_cast<size_t>(direction)] = shouldBeMoving;
}

void EntityController::drawNameTag(
    WindowManager& window,
    AssetManager& assets,
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    float alpha
) {
    if (!entity || !nametag) {
        return;
    }
    b2Vec2 pos = getNametagWorldPos(alpha);
    Drawing::text(
        nametag.get(),
        window,
        pos,
        cameraScale,
        cameraOffsetPixels,
        assets.TextRenderScale,
        assets.TextWorldSizeMultiplier
    );
}

b2Vec2
EntityController::getNametagWorldSize(float textRenderScale, float textWorldSizeMultiplier) const {
    return Drawing::getTextWorldSize(nametag.get(), textRenderScale, textWorldSizeMultiplier);
}

b2Vec2 EntityController::getNametagWorldPos(float alpha) const {
    if (!nametag || !entity) {
        return b2Vec2{0.f, 0.f};
    }
    b2Vec2 pos = entity->getInterpolatedPosition(alpha);
    b2BodyId bodyId = entity->getBodyId();
    b2AABB aabb = b2Body_ComputeAABB(bodyId);
    pos.y += (aabb.upperBound.y - aabb.lowerBound.y);
    return pos;
}