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
    SDL_FColor color;
    b2BodyId bodyId;
    b2Polygon polygon;
    b2Vec2 previousPosition;
    float previousAngle;

  public:
    Entity(b2WorldId world, b2Polygon polygon, b2Vec2 position, SDL_Color color, bool isStatic);
    Entity(
        b2WorldId world,
        b2Polygon polygon,
        b2Vec2 position,
        SDL_Color color,
        bool isStatic,
        b2BodyDef bodyDef,
        b2ShapeDef shapeDef
    );
    ~Entity();

    const bool isStatic;

    b2BodyId getBodyId() const;
    b2Polygon getPolygon() const;
    SDL_Color getColor() const;
    b2Vec2 getPosition() const;
    b2Vec2 getInterpolatedPosition(float alpha) const;
    b2Rot getInterpolatedRotation(float alpha) const;
    void setColor(SDL_Color color);

    void savePreviousState();

    void draw(WindowManager& window, float alpha) const;
    void teleport(b2Vec2 location);
};