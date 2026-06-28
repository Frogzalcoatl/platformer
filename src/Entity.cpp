#include "Entity.hpp"
#include "Colors.hpp"

Entity::Entity(b2WorldId world, b2Vec2 size, b2Vec2 position, SDL_Color color, bool isStatic)
    : Entity(world, size, position, color, isStatic, b2DefaultBodyDef(), b2DefaultShapeDef()) {}

Entity::Entity(
    b2WorldId world, b2Vec2 size, b2Vec2 position, SDL_Color color, bool isStatic,
    b2BodyDef bodyDef, b2ShapeDef shapeDef
)
    : isStatic(isStatic) {
    bodyDef.position = position;
    if (!this->isStatic) {
        bodyDef.type = b2_dynamicBody;
    }
    setColor(color);
    bodyId = b2CreateBody(world, &bodyDef);
    polygon = b2MakeBox(size.x, size.y);
    b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
    spawnPoint = position;
}

Entity::~Entity() { b2DestroyBody(bodyId); }

b2BodyId Entity::getBodyId() { return bodyId; }

b2Polygon Entity::getPolygon() { return polygon; }

void Entity::setColor(SDL_Color c) { color = colorToFColor(c); }

static SDL_FPoint scaleB2Point(WindowManager* window, b2Transform transform, b2Vec2 point) {
    b2Vec2 worldPosition = b2TransformPoint(transform, point);
    float scaleFactor = window->getScaleFactor();
    WindowDimensions offset = window->getOffsetPixels();
    worldPosition.x *= scaleFactor;
    worldPosition.y *= scaleFactor;
    worldPosition.y *= -1.f;
    worldPosition.x += offset.x;
    worldPosition.y += offset.y;
    return SDL_FPoint{worldPosition.x, worldPosition.y};
}

void Entity::draw(WindowManager* window) const {
    b2Transform transform = b2Body_GetTransform(bodyId);
    SDL_Vertex sdlVertices[B2_MAX_POLYGON_VERTICES] = {0};
    for (int i = 0; i < polygon.count; i++) {
        sdlVertices[i].color = color;
        sdlVertices[i].position = scaleB2Point(window, transform, polygon.vertices[i]);
    }
    const int indices[] = {0, 1, 2, 2, 3, 0};
    SDL_RenderGeometry(window->sdlRenderer, NULL, sdlVertices, polygon.count, indices, 6);
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
    if (movement[EntityMovement_Down]) {
        targetVelocity.y -= downwardAcceleration;
    }
    if (movement[EntityMovement_Left]) {
        targetVelocity.x -= maxHorizontalSpeed;
    }
    if (movement[EntityMovement_Right]) {
        targetVelocity.x += maxHorizontalSpeed;
    }
    velocity.x = velocity.x + (targetVelocity.x - velocity.x) * horizontalAcceleration;
    velocity.y += targetVelocity.y;
    b2Body_SetLinearVelocity(bodyId, velocity);
}

void Entity::jump() {
    b2Body_ApplyLinearImpulseToCenter(bodyId, b2Vec2{0.f, jumpForceNewtons}, true);
}

void Entity::teleport(b2Vec2 location) {
    b2Body_SetLinearVelocity(
        bodyId, b2Vec2{0.f, -0.01f}
    ); // y not set to 0.f since that results in floating entity until its ineracted with.
    b2Body_SetTransform(bodyId, location, b2Body_GetRotation(bodyId));
}

void Entity::respawn() { teleport(spawnPoint); }