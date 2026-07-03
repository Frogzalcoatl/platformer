#include "Entity.hpp"
#include "Colors.hpp"
#include "Drawing.hpp"
#include <cmath>

Entity::Entity(b2WorldId world, b2Polygon polygon, b2Vec2 position, SDL_Color color, bool isStatic)
    : Entity(world, polygon, position, color, isStatic, b2DefaultBodyDef(), b2DefaultShapeDef()) {
}

Entity::Entity(
    b2WorldId world,
    b2Polygon polygon,
    b2Vec2 position,
    SDL_Color color,
    bool isStatic,
    b2BodyDef bodyDef,
    b2ShapeDef shapeDef
)
    : polygon(polygon), isStatic(isStatic) {
    bodyDef.position = position;
    if (!isStatic) {
        bodyDef.type = b2_dynamicBody;
    }
    setColor(color);
    bodyId = b2CreateBody(world, &bodyDef);
    b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
    previousPosition = position;
    previousAngle = b2Rot_GetAngle(bodyDef.rotation);
}

Entity::~Entity() {
    b2DestroyBody(bodyId);
}

b2BodyId Entity::getBodyId() const {
    return bodyId;
}

b2Polygon Entity::getPolygon() const {
    return polygon;
}

void Entity::setColor(SDL_Color c) {
    color = colorToFColor(c);
}

SDL_Color Entity::getColor() const {
    return fColorToColor(color);
}

b2Vec2 Entity::getPosition() const {
    return b2Body_GetPosition(bodyId);
}

b2Vec2 Entity::getInterpolatedPosition(float alpha) const {
    b2Vec2 currentPos = b2Body_GetPosition(bodyId);
    b2Vec2 interpolatedPos;
    interpolatedPos.x = previousPosition.x * (1.f - alpha) + currentPos.x * alpha;
    interpolatedPos.y = previousPosition.y * (1.f - alpha) + currentPos.y * alpha;
    return interpolatedPos;
}

b2Rot Entity::getInterpolatedRotation(float alpha) const {
    b2Rot currentRot = b2Body_GetRotation(bodyId);
    b2Rot previousRot = b2MakeRot(previousAngle);
    b2Rot interpolatedRot = b2NLerp(previousRot, currentRot, alpha);
    return interpolatedRot;
}

void Entity::savePreviousState() {
    previousPosition = b2Body_GetPosition(bodyId);
    previousAngle = b2Rot_GetAngle(b2Body_GetRotation(bodyId));
}

void Entity::draw(
    WindowManager& window, float alpha, float scaleFactor, WindowDimensions offsetPixels
) const {
    b2Transform transform;
    transform.p = getInterpolatedPosition(alpha);
    transform.q = getInterpolatedRotation(alpha);
    Drawing::polygon(polygon, window, transform, scaleFactor, offsetPixels, color);
}

void Entity::teleport(b2Vec2 location) {
    b2Body_SetLinearVelocity(
        bodyId, b2Vec2{0.f, -0.01f}
    ); // y not set to 0.f since that results in floating entity until its interacted with.
    b2Body_SetAngularVelocity(bodyId, 0.f);
    // b2Rot_identity is default rotation
    b2Body_SetTransform(bodyId, location, b2Rot_identity);
    previousPosition = location;
    previousAngle = b2Rot_GetAngle(b2Rot_identity);
}