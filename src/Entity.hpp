#pragma once
#include "WindowManager.hpp"
#include <SDL3/SDL.h>
#include <array>
#include <box2d/box2d.h>
#include <optional>
#include <vector>

enum class EntityMovement : uint8_t {
    Up,
    Down,
    Left,
    Right,
    EntityMovementCount
};

class Entity {
  private:
    b2BodyId bodyId;
    b2Polygon polygon;
    b2Vec2 previousPosition;
    float previousAngle;
    SDL_Texture* texture;
    std::optional<b2Vec2> textureSize;

  public:
    Entity(
        b2WorldId world,
        b2Polygon polygon,
        b2Vec2 position,
        bool isStatic,
        SDL_Texture* texture,
        std::optional<b2Vec2> textureSize = std::nullopt
    );
    Entity(
        b2WorldId world,
        b2Polygon polygon,
        b2Vec2 position,
        bool isStatic,
        SDL_Texture* texture,
        std::optional<b2Vec2> textureSize,
        b2BodyDef bodyDef,
        b2ShapeDef shapeDef
    );
    ~Entity();

    const bool isStatic;

    b2BodyId getBodyId() const;
    b2Polygon getPolygon() const;
    b2Vec2 getPosition() const;
    b2Vec2 getInterpolatedPosition(float alpha) const;
    b2Rot getInterpolatedRotation(float alpha) const;

    void savePreviousState();

    void draw(WindowManager& window, float alpha, float scaleFactor, WindowVec2 offsetPixels) const;
    void drawHitbox(
        WindowManager& window, float alpha, float scaleFactor, WindowVec2 offsetPixels
    ) const;
    void teleport(b2Vec2 location);
};