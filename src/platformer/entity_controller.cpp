#include "platformer/entity_controller.h"
#include "assets/asset_paths.h"
#include <cassert>

static std::optional<EntityMovement> input_verb_to_direction(InputVerb verb) {
    switch (verb) {
    case InputVerb::up:
        return EntityMovement::up;
    case InputVerb::down:
        return EntityMovement::down;
    case InputVerb::left:
        return EntityMovement::left;
    case InputVerb::right:
        return EntityMovement::right;
    default:
        return std::nullopt;
    }
}

EntityController::EntityController(Entity& entity) : entity_(&entity) {
}

void EntityController::set_entity(Entity& new_entity) {
    entity_ = &new_entity;
}

void EntityController::clear_entity() {
    entity_ = nullptr;
}

Entity* EntityController::get_entity() const {
    return entity_;
}

void EntityController::update() {
    if (!entity_) {
        return;
    }
    b2BodyId body_id = entity_->get_body_id();
    b2Vec2 velocity = b2Body_GetLinearVelocity(body_id);
    b2Vec2 target_velocity = {0.f, 0.f};

    if (movement[static_cast<size_t>(EntityMovement::down)]) {
        target_velocity.y -= downward_acceleration;
    }
    if (movement[static_cast<size_t>(EntityMovement::left)]) {
        target_velocity.x -= horizontal_speed;
    }
    if (movement[static_cast<size_t>(EntityMovement::right)]) {
        target_velocity.x += horizontal_speed;
    }
    if (is_sprinting) {
        target_velocity.x *= sprint_multiplier;
        target_velocity.y *= sprint_multiplier;
    }
    velocity.x = velocity.x + (target_velocity.x - velocity.x) * horizontal_acceleration;
    velocity.y += target_velocity.y;
    b2Body_SetLinearVelocity(body_id, velocity);
}

void EntityController::jump() {
    if (!entity_) {
        return;
    }
    b2BodyId body_id = entity_->get_body_id();
    b2Vec2 velocity = b2Body_GetLinearVelocity(body_id);
    b2Body_SetLinearVelocity(body_id, b2Vec2{velocity.x, 0.f});
    b2Body_ApplyLinearImpulseToCenter(body_id, b2Vec2{0.f, jump_force_newtons}, true);
}

void EntityController::respawn() {
    if (!entity_) {
        return;
    }
    entity_->teleport(spawn_point);
}

void EntityController::reset_input() {
    for (size_t i = 0; i < movement.size(); i++) {
        movement[i] = false;
    }
    is_sprinting = false;
}

void EntityController::handle_input(game_event_types::Input event, Camera* camera, float alpha) {
    assert(event.state == InputState::pressed || event.state == InputState::released);
    assert(event.verb < InputVerb::verb_count);
    if (!entity_) {
        return;
    }
    if (event.state == InputState::pressed) {
        if (event.verb == InputVerb::respawn) {
            respawn();
            if (camera && camera->entity_to_follow == entity_) {
                camera->center_on_entity(alpha);
            }
        } else if (event.verb == InputVerb::jump) {
            jump();
            float pitch = SDL_randf() * (1.25f - 1.f) + 1.f;
            game_events::push(game_event_types::PlaySound{asset_paths::sounds::jump, 100, pitch});
        }
    }
    if (event.verb == InputVerb::sprint) {
        is_sprinting = event.state == InputState::pressed;
    }
    std::optional<EntityMovement> direction_opt = input_verb_to_direction(event.verb);
    if (!direction_opt.has_value()) {
        return;
    }
    EntityMovement direction = direction_opt.value();
    bool should_be_moving = event.state == InputState::pressed;
    movement[static_cast<size_t>(direction)] = should_be_moving;
}