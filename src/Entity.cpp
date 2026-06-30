#include "Entity.hpp"
#include "Colors.hpp"
#include "Drawing.hpp"
#include <cmath>

Entity::Entity(b2WorldId world, b2Polygon polygon, b2Vec2 position, SDL_Color color, bool isStatic)
    : Entity(world, polygon, position, color, isStatic, b2DefaultBodyDef(), b2DefaultShapeDef()) {}

Entity::Entity(
    b2WorldId world, b2Polygon polygonArg, b2Vec2 position, SDL_Color color, bool isStatic,
    b2BodyDef bodyDef, b2ShapeDef shapeDef
)
    : isStatic(isStatic) {
    bodyDef.position = position;
    if (!isStatic) {
        bodyDef.type = b2_dynamicBody;
    }
    setColor(color);
    bodyId = b2CreateBody(world, &bodyDef);
    polygon = polygonArg;
    b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
    spawnPoint = position;
}

Entity::~Entity() { b2DestroyBody(bodyId); }

b2BodyId Entity::getBodyId() const { return bodyId; }

b2Polygon Entity::getPolygon() const { return polygon; }

void Entity::setColor(SDL_Color c) { color = colorToFColor(c); }

SDL_Color Entity::getColor() const { return fColorToColor(color); }

b2Vec2 Entity::getPosition() const { return b2Body_GetPosition(bodyId); }

void Entity::draw(WindowManager& window) const {
    b2Transform transform = b2Body_GetTransform(bodyId);
    Drawing::polygon(polygon, transform, window, color);
}

void Entity::update() {
    if (isStatic) {
        return;
    }
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

void Entity::jump() {
    b2Vec2 velocity = b2Body_GetLinearVelocity(bodyId);
    b2Body_SetLinearVelocity(bodyId, b2Vec2{velocity.x, 0.f});
    b2Body_ApplyLinearImpulseToCenter(bodyId, b2Vec2{0.f, jumpForceNewtons}, true);
}

void Entity::teleport(b2Vec2 location) {
    b2Body_SetLinearVelocity(
        bodyId, b2Vec2{0.f, -0.01f}
    ); // y not set to 0.f since that results in floating entity until its interacted with.
    b2Body_SetAngularVelocity(bodyId, 0.f);
    // b2Rot_identity is default rotation
    b2Body_SetTransform(bodyId, location, b2Rot_identity);
}

void Entity::respawn() { teleport(spawnPoint); }