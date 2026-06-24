#include "gameObject.hpp"

static b2BodyDef defaultBodyDef = b2DefaultBodyDef();
static b2ShapeDef defaultShapeDef = b2DefaultShapeDef();

GameObject::GameObject(b2WorldId world, b2Vec2 size, b2Vec2 position, std::optional<b2BodyDef> bodyDefOpt,
                       std::optional<b2ShapeDef> shapeDefOpt, std::optional<SDL_FColor> colorOpt) {
    b2BodyDef bodyDef = bodyDefOpt.value_or(b2DefaultBodyDef());
    bodyDef.position = position;
    b2ShapeDef shapeDef = shapeDefOpt.value_or(b2DefaultShapeDef());
    color = colorOpt.value_or(SDL_FColor{1.0f, 1.0f, 1.0f, 1.0f});
    bodyId = b2CreateBody(world, &bodyDef);
    polygon = b2MakeBox(size.x, size.y);
    b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
}

SDL_FPoint scaleB2Point(WindowManager* window, b2Transform transform, b2Vec2 point) {
    b2Vec2 worldPosition = b2TransformPoint(transform, point);
    int scaleFactor = window->getScaleFactor();
    WindowDimensions offset = window->getOffset();
    worldPosition.x *= scaleFactor;
    worldPosition.y *= scaleFactor;
    worldPosition.y *= -1.f;
    worldPosition.x += offset.x;
    worldPosition.y += offset.y;
    return SDL_FPoint{worldPosition.x, worldPosition.y};
}

void GameObject::draw(WindowManager* window) {
    b2Transform transform = b2Body_GetTransform(bodyId);
    SDL_Vertex sdlVertices[B2_MAX_POLYGON_VERTICES] = {0};
    for (int i = 0; i < polygon.count; i++) {
        sdlVertices[i].color = color;
        sdlVertices[i].position = scaleB2Point(window, transform, polygon.vertices[i]);
    }
    const int indices[] = {0, 1, 2, 2, 3, 0};
    SDL_RenderGeometry(window->sdlRenderer, NULL, sdlVertices, polygon.count, indices, 6);
}