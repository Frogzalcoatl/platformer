#pragma once
#include "entities/Entity.hpp"
#include "events/EventQueue.hpp"
#include "platformer/Camera.hpp"
#include <optional>

class EntityController {
  private:
    Entity* entity;

  public:
    EntityController() = default;
    EntityController(Entity& entity);

    void setEntity(Entity& entity);

    void clearEntity();

    Entity* getEntity() const;

    void update();

    void jump();

    void respawn();

    void resetInput();

    void handleInput(GameEventTypes::Input event, Camera* camera, float alpha);

    b2Vec2 spawnPoint = {0.f, 0.f};
    float jumpForceNewtons = 160.f;
    float horizontalSpeed = 10.f;
    float horizontalAcceleration = 0.1f;
    float downwardAcceleration = 2.5f;
    std::array<bool, static_cast<size_t>(EntityMovement::EntityMovementCount)> movement = {false};
    bool isSprinting = false;
    float sprintMultiplier = 2.f;
};