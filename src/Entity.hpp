#pragma once
#include "WindowManager.hpp"
#include <SDL3/SDL.h>
#include <array>
#include <box2d/box2d.h>
#include <optional>
#include <vector>

enum class EntityMovement : uint8_t { Up, Down, Left, Right, IsRunning, Count };

class Entity {
  private:
    SDL_FColor color;
    b2BodyId bodyId;
    b2Polygon polygon;

  public:
    Entity(b2WorldId world, b2Vec2 size, b2Vec2 position, SDL_Color color, bool isStatic);
    Entity(
        b2WorldId world, b2Vec2 size, b2Vec2 position, SDL_Color color, bool isStatic,
        b2BodyDef bodyDef, b2ShapeDef shapeDef
    );
    ~Entity();
    b2BodyId getBodyId();
    b2Polygon getPolygon();
    const bool isStatic;
    b2Vec2 spawnPoint = {0.f, 0.f};
    const float jumpForceNewtons = 20.f;
    const float maxHorizontalSpeed = 20.f;
    const float horizontalAcceleration = 0.1f;
    const float downwardAcceleration = 5.f;
    std::array<bool, static_cast<size_t>(EntityMovement::Count)> movement = {false};
    void draw(WindowManager& window) const;
    void jump();
    void update();
    void teleport(b2Vec2 location);
    void respawn();
    void setColor(SDL_Color color);
    SDL_Color getColor();
};