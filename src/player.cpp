#include "player.hpp"

Player::Player(b2WorldId world, b2Vec2 size, b2Vec2 position, SDL_FColor color)
    : GameObject(world, size, position, prepareBodyDef(), prepareShapeDef(), color) {}

void Player::handleSDLKeyEvent(SDL_Event* event) {
    bool setMovementTo = false;
    if (event->type == SDL_EVENT_KEY_DOWN) {
        setMovementTo = true;
    } else if (event->type != SDL_EVENT_KEY_UP) {
        return;
    }
    if (event->key.repeat) {
        return;
    }
    switch (event->key.scancode) {
    case SDL_SCANCODE_W:
        movement.up = setMovementTo;
        break;
    case SDL_SCANCODE_S:
        movement.down = setMovementTo;
        break;
    case SDL_SCANCODE_A:
        movement.left = setMovementTo;
        break;
    case SDL_SCANCODE_D:
        movement.right = setMovementTo;
        break;
    }
    if (event->type == SDL_EVENT_KEY_DOWN && !event->key.repeat) {
        if (event->key.scancode == SDL_SCANCODE_W) {
            jump();
        } else if (event->key.scancode == SDL_SCANCODE_R) {
            b2Body_SetTransform(bodyId, {0.f, 4.f}, b2Body_GetRotation(bodyId));
        }
    }
}

const float MovementForceNewtons = 20.f;

void Player::update(void) {
    // Controls
    b2Vec2 velocity = b2Body_GetLinearVelocity(bodyId);
    velocity.x = 0.f;
    if (movement.down) {
        velocity.y -= MovementForceNewtons;
    }
    if (movement.left) {
        velocity.x -= MovementForceNewtons;
    }
    if (movement.right) {
        velocity.x += MovementForceNewtons;
    }
    b2Body_SetLinearVelocity(bodyId, velocity);
}

const float JumpForceNewtons = 15.f;

void Player::jump(void) { b2Body_ApplyLinearImpulseToCenter(bodyId, {0.f, JumpForceNewtons}, true); }