#pragma once
#include "windowManager.hpp"
#include <SDL3/SDL.h>
#include <array>
#include <box2d/box2d.h>
#include <optional>
#include <vector>

enum EntityMovement : uint8_t {
    EntityMovement_Up,
    EntityMovement_Down,
    EntityMovement_Left,
    EntityMovement_Right,
    EntityMovement_IsRunning,
    EntityMovement_Count
};

class Entity {
  private:
    SDL_FColor color;

  public:
    Entity(b2WorldId world, b2Vec2 size, b2Vec2 position, SDL_Color color, bool isStatic);
    Entity(
        b2WorldId world, b2Vec2 size, b2Vec2 position, SDL_Color color, bool isStatic,
        b2BodyDef bodyDef, b2ShapeDef shapeDef
    );
    ~Entity();
    b2BodyId bodyId;
    b2Polygon polygon;
    const bool isStatic;
    b2Vec2 spawnPoint = {0.f, 0.f};
    const float jumpForceNewtons = 15.f;
    const float maxHorizontalSpeed = 20.f;
    const float horizontalAcceleration = 0.25f;
    const float downwardAcceleration = 5.f;
    std::array<bool, EntityMovement_Count> movement = {false};
    void draw(WindowManager* window) const;
    void jump();
    void update();
    void teleport(b2Vec2 location);
    void respawn();
    void setColor(SDL_Color);
};