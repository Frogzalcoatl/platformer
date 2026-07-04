#pragma once
#include "Camera.hpp"
#include "Entity.hpp"
#include "Events.hpp"
#include "InputManager.hpp"
#include <optional>

class EntityController {
  private:
    Entity* entity;

  public:
    EntityController() = default;
    EntityController(Entity& entity, std::optional<SDL_JoystickID> joystickId = std::nullopt);
    std::optional<SDL_JoystickID> joystickId;
    b2Vec2 spawnPoint = {0.f, 0.f};
    float jumpForceNewtons = 160.f;
    float horizontalSpeed = 10.f;
    float horizontalAcceleration = 0.1f;
    float downwardAcceleration = 2.5f;
    std::array<bool, static_cast<size_t>(EntityMovement::EntityMovementCount)> movement = {false};
    bool isSprinting = false;
    float sprintMultiplier = 2.f;
    void setEntity(Entity& entity);
    void clearEntity();
    Entity* getEntity() const;
    void update();
    void jump();
    void respawn();
    void handleInput(GameEventTypes::Input event, Camera* camera);
    void resetMovement();
};