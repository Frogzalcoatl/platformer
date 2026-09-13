#pragma once
#include "entities/entity.h"
#include "platformer/camera.h"
#include "platformer/game_events.h"
#include <array>

class EntityController {
  private:
    Entity* entity_ = nullptr;

  public:
    EntityController() = default;
    EntityController(Entity& entity);

    void set_entity(Entity& entity);
    void clear_entity();
    Entity* get_entity() const;

    void update();
    void jump();
    void respawn();
    void reset_input();
    void handle_input(game_event_types::Input event, Camera* camera, float alpha);

    b2Vec2 spawn_point = {0.f, 0.f};
    float jump_force_newtons = 160.f;
    float horizontal_speed = 10.f;
    float horizontal_acceleration = 0.1f;
    float downward_acceleration = 2.5f;
    std::array<bool, static_cast<size_t>(EntityMovement::entity_movement_count)> movement = {false};
    bool is_sprinting = false;
    float sprint_multiplier = 2.f;
};