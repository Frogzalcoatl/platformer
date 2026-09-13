#pragma once
#include <array>
#include <string_view>

namespace asset_paths {

namespace fonts {
inline constexpr std::string_view consolas = "fonts/consola.ttf";
}

namespace sounds {
inline constexpr std::string_view jump = "sounds/jump.wav";
inline constexpr std::string_view click = "sounds/click.wav";
inline constexpr std::string_view hover = "sounds/hover.wav";
inline constexpr std::string_view edit = "sounds/edit.wav";
}

namespace textures {
inline constexpr std::string_view missing = "textures/missing.png";
inline constexpr std::string_view player = "textures/entities/player.png";
inline constexpr std::string_view log = "textures/entities/log.png";

enum class TileTypes : size_t {
    air,
    grass,
    dirt,
    stone,
    tile_count
};

inline constexpr std::array<std::string_view, static_cast<size_t>(TileTypes::tile_count)>
    tile_paths = {
        "", "textures/tiles/grass.png", "textures/tiles/dirt.png", "textures/tiles/stone.png"
};
}

}