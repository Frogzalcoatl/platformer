#pragma once
#include "AssetManager.hpp"
#include "WindowManager.hpp"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

struct Vec2Int {
    int x = 0;
    int y = 0;
};

class Tile {
  private:
    Vec2Int position;
    SDL_Texture* texture;

  public:
    Tile(Vec2Int position, AssetManager& assets, GameAssets::Textures textureId);
    void draw(WindowManager& window);
};