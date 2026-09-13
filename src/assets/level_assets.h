#pragma once
#include "assets/asset_manager.h"
#include "assets/asset_paths.h"
#include <optional>
#include <vector>

struct LevelAsset {
    std::string_view relative_path;
    AssetTypes type;
    std::optional<FontInfo> font_info = std::nullopt;
    bool should_be_predecoded_audio = false;
    bool operator==(const LevelAsset& other) const = default;
};

using LevelAssetsVector = std::vector<LevelAsset>;

namespace level_assets {
inline LevelAssetsVector template_assets = {
    {asset_paths::textures::player, AssetTypes::texture},
    {asset_paths::textures::log, AssetTypes::texture},
    {asset_paths::textures::tile_paths[static_cast<size_t>(asset_paths::textures::TileTypes::dirt)],
     AssetTypes::texture},
    {asset_paths::textures::tile_paths
         [static_cast<size_t>(asset_paths::textures::TileTypes::grass)],
     AssetTypes::texture},
    {asset_paths::textures::tile_paths
         [static_cast<size_t>(asset_paths::textures::TileTypes::stone)],
     AssetTypes::texture},
    {asset_paths::fonts::consolas, AssetTypes::font_sdl, FontInfo{20.f}},
    {asset_paths::sounds::jump, AssetTypes::audio, std::nullopt, true}
};
}