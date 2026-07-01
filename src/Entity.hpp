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
    IsRunning,
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
    b2Vec2 spawnPoint = {0.f, 0.f};
    const float jumpForceNewtons = 20.f;
    const float horizontalSpeed = 10.f;
    const float horizontalAcceleration = 0.1f;
    const float downwardAcceleration = 2.5f;
    std::array<bool, static_cast<size_t>(EntityMovement::EntityMovementCount)> movement = {false};
    bool isSprinting = false;
    float sprintMultiplier = 2.f;

    b2BodyId getBodyId() const;
    b2Polygon getPolygon() const;
    SDL_Color getColor() const;
    b2Vec2 getPosition() const;
    b2Vec2 getInterpolatedPosition(float alpha) const;
    b2Rot getInterpolatedRotation(float alpha) const;
    void setColor(SDL_Color color);

    void savePreviousState();

    void draw(WindowManager& window, float alpha) const;
    void jump();
    void update();
    void teleport(b2Vec2 location);
    void respawn();
};