#include "Entity.hpp"
#include "Colors.hpp"
#include "Drawing.hpp"
#include <cmath>

Entity::Entity(
    b2WorldId world,
    b2Polygon polygon,
    b2Vec2 position,
    bool isStatic,
    b2BodyDef bodyDef,
    b2ShapeDef shapeDef,
    SDL_FColor hitboxColor,
    SDL_Texture* texture,
    std::optional<b2Vec2> textureSize
)
    : polygon(polygon), texture(texture), textureSize(textureSize), isStatic(isStatic),
      hitboxColor(hitboxColor) {
    bodyDef.position = position;
    if (!isStatic) {
        bodyDef.type = b2_dynamicBody;
    }
    bodyId = b2CreateBody(world, &bodyDef);
    b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
    previousPosition = position;
    previousAngle = b2Rot_GetAngle(bodyDef.rotation);
}

Entity::~Entity() {
    b2DestroyBody(bodyId);
}

const b2BodyId& Entity::getBodyId() const {
    return bodyId;
}

const b2Polygon& Entity::getPolygon() const {
    return polygon;
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

bool Entity::draw(
    WindowManager& window, float alpha, float cameraScale, WindowVec2 cameraOffsetPixels
) const {
    if (!texture) {
        return false;
    }
    b2Vec2 pos = getInterpolatedPosition(alpha);
    b2Rot rot = getInterpolatedRotation(alpha);
    Drawing::texture(
        texture,
        window,
        pos,
        textureSize.value_or(b2Vec2{1.f, 1.f}),
        cameraScale,
        cameraOffsetPixels,
        Drawing::b2RotToSdlAngle(rot)
    );
    return true;
}

void Entity::drawHitbox(
    WindowManager& window, float alpha, float cameraScale, WindowVec2 cameraOffsetPixels
) const {
    b2Transform transform;
    transform.p = getInterpolatedPosition(alpha);
    transform.q = getInterpolatedRotation(alpha);
    Drawing::polygonBorders(
        polygon, window, transform, cameraScale, cameraOffsetPixels, hitboxColor
    );
}

void Entity::teleport(b2Vec2 location) {
    b2Body_SetLinearVelocity(bodyId, b2Vec2{0.f, 0.f});
    b2Body_SetAngularVelocity(bodyId, 0.f);
    // b2Rot_identity is default rotation
    b2Body_SetTransform(bodyId, location, b2Rot_identity);
    b2Body_SetAwake(bodyId, true);
    previousPosition = location;
    previousAngle = b2Rot_GetAngle(b2Rot_identity);
}