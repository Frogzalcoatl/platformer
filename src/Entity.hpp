#pragma once
#include "Colors.hpp"
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
    b2Vec2 positionLastRealFrame;
    float angleLastRealFrame;
    SDL_Texture* texture;
    std::optional<b2Vec2> textureSize;

  public:
    Entity(
        b2WorldId world,
        b2Polygon polygon,
        b2Vec2 position,
        b2BodyDef bodyDef = b2DefaultBodyDef(),
        b2ShapeDef shapeDef = b2DefaultShapeDef(),
        SDL_FColor hitboxColor = colorToFColor(Colors::Yellow),
        SDL_Texture* texture = nullptr,
        std::optional<b2Vec2> textureSize = std::nullopt
    );
    ~Entity();

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    SDL_FColor hitboxColor;

    const b2BodyId& getBodyId() const;
    const b2Polygon& getPolygon() const;
    b2Vec2 getPosition() const;
    b2Vec2 getInterpolatedPosition(float alpha) const;
    b2Rot getInterpolatedRotation(float alpha) const;

    void savePreviousState();

    bool draw(
        WindowManager& window, float alpha, float cameraScale, WindowVec2 cameraOffsetPixels
    ) const;
    void drawHitbox(
        WindowManager& window, float alpha, float cameraScale, WindowVec2 cameraOffsetPixels
    ) const;
    void teleport(b2Vec2 location);
};