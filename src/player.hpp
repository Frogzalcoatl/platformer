#pragma once
#include "gameObject.hpp"

struct PlayerMovement {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};

class Player : public GameObject {
  private:
    void jump(void);

    static b2BodyDef prepareBodyDef() {
        b2BodyDef def = b2DefaultBodyDef();
        def.type = b2_dynamicBody;
        def.linearDamping = 0.5f;
        return def;
    }

    static b2ShapeDef prepareShapeDef() {
        b2ShapeDef def = b2DefaultShapeDef();
        def.density = 1.f;
        def.material.friction = 0.3f;
        return def;
    }

  public:
    Player(b2WorldId world, b2Vec2 size, b2Vec2 position, SDL_FColor color);

    PlayerMovement movement;

    void handleSDLKeyEvent(SDL_Event* event);
    void update(void);
};