#pragma once
#include "windowManager.hpp"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <optional>
#include <vector>

class GameObject {
  public:
    inline static std::vector<GameObject*> instances;
    b2BodyId bodyId;
    b2Polygon polygon;
    b2WorldId world;
    SDL_FColor color;
    GameObject::GameObject(b2WorldId world, b2Vec2 size, b2Vec2 position, std::optional<b2BodyDef> bodyDefOpt,
                           std::optional<b2ShapeDef> shapeDefOpt, std::optional<SDL_FColor> colorOpt);
    void draw(WindowManager* window);
};