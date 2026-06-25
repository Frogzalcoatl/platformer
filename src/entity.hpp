#pragma once
#include "windowManager.hpp"
#include <SDL3/SDL.h>
#include <array>
#include <box2d/box2d.h>
#include <optional>
#include <vector>

enum EntityMovement {
    EntityMovement_Up,
    EntityMovement_Down,
    EntityMovement_Left,
    EntityMovement_Right,
    EntityMovement_IsRunning,
    EntityMovement_Count
};

namespace DynamicEntityDefaults {
constexpr float LinearDamping = 0.5;
constexpr float Density = 1.f;
constexpr float Friction = 0.3f;
}; // namespace DynamicEntityDefaults

class Entity {
  private:
    SDL_FColor color;
    const bool isStatic;

  public:
    Entity(b2WorldId world, b2Vec2 size, b2Vec2 position, SDL_Color color, bool isStatic);
    Entity(
        b2WorldId world, b2Vec2 size, b2Vec2 position, SDL_Color color, bool isStatic,
        b2BodyDef bodyDef, b2ShapeDef shapeDef
    );
    b2BodyId bodyId;
    b2Polygon polygon;
    b2Vec2 spawnPoint = {0.f, 0.f};
    float jumpForceNewtons = 15.f;
    float maxSpeed = 20.f;
    float movementAcceleration = 0.25f;
    float downwardSpeed = 5.f;
    std::array<bool, EntityMovement_Count> movement = {false};
    void draw(WindowManager* window);
    void jump(void);
    void update(void);
    void teleport(b2Vec2 location);
    void respawn(void);
    void setColor(SDL_Color);
    bool getIsStatic(void);
};