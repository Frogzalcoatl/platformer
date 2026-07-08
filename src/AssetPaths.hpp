#pragma once
#include <array>
#include <string_view>

namespace AssetPaths {

namespace Fonts {
inline constexpr std::string_view Xsku = "fonts/xsku.ttf";
inline constexpr std::string_view Consolas = "fonts/consola.ttf";
}

namespace Sounds {
inline constexpr std::string_view Fart = "sounds/far.wav";
inline constexpr std::string_view Jump = "sounds/jump.wav";
}

namespace Textures {
inline constexpr std::string_view Missing = "textures/missing.png";
inline constexpr std::string_view Player = "textures/entities/player.png";
inline constexpr std::string_view Log = "textures/entities/log.png";
enum class TileTypes : size_t {
    Air,
    Grass,
    Dirt,
    Stone,
    TileCount
};
inline constexpr std::array<std::string_view, static_cast<size_t>(TileTypes::TileCount)> TilePaths =
    {"", "textures/tiles/grass.png", "textures/tiles/dirt.png", "textures/tiles/stone.png"};
}
}