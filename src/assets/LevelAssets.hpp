#pragma once
#include "assets/AssetManager.hpp"
#include "assets/AssetPaths.hpp"
#include <optional>

struct LevelAsset {
    std::string_view relativePath;
    AssetTypes type;
    std::optional<FontInfo> fontInfo = std::nullopt;
    bool shouldBePredecodedAudio = false;
    bool operator==(const LevelAsset& other) const = default;
};

using LevelAssetsVector = std::vector<LevelAsset>;

namespace LevelAssets {
inline LevelAssetsVector Template = {
    {AssetPaths::Textures::Player, AssetTypes::Texture},
    {AssetPaths::Textures::Log, AssetTypes::Texture},
    {AssetPaths::Textures::TilePaths[static_cast<size_t>(AssetPaths::Textures::TileTypes::Dirt)],
     AssetTypes::Texture},
    {AssetPaths::Textures::TilePaths[static_cast<size_t>(AssetPaths::Textures::TileTypes::Grass)],
     AssetTypes::Texture},
    {AssetPaths::Textures::TilePaths[static_cast<size_t>(AssetPaths::Textures::TileTypes::Stone)],
     AssetTypes::Texture},
    {AssetPaths::Fonts::Consolas, AssetTypes::FontSdl, FontInfo{20.f}},
    {AssetPaths::Sounds::Jump, AssetTypes::Audio, std::nullopt, true}
};
}