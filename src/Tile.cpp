#include "Tile.hpp"
#include "Drawing.hpp"

Tile::Tile(Vec2Int position, AssetManager& assets, GameAssets::Textures textureId)
    : position(position) {
    texture = assets.getTexture(textureId);
}

void Tile::draw(WindowManager& window) {
    if (!texture) {
        return;
    }

    Drawing::texture(
        window,
        texture,
        b2Vec2{static_cast<float>(position.x), static_cast<float>(position.y)},
        b2Vec2{1.f, 1.f},
        45.0,
        SDL_FLIP_HORIZONTAL
    );
}