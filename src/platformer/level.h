#pragma once
#include "assets/asset_paths.h"
#include "assets/colors.h"
#include "assets/level_assets.h"
#include "platformer/camera.h"
#include "platformer/entity_controller.h"
#include "platformer/game_events.h"
#include "system/audio_manager.h"
#include "system/input_manager.h"
#include <box2d/box2d.h>
#include <optional>
#include <vector>

struct LevelDimensions {
    size_t width;
    size_t height;
};

struct LevelDrawInfo {
    size_t tiles = 0;
    size_t entities = 0;
};

struct LevelDrawDimensions {
    size_t min_x = 0;
    size_t max_x = 0;
    size_t min_y = 0;
    size_t max_y = 0;
};

struct Player {
    InputSource source;
    std::unique_ptr<EntityController> controller;
};

using LevelTileVector = std::vector<std::vector<asset_paths::textures::TileTypes>>;
using EntitiesVector = std::vector<std::unique_ptr<Entity>>;

class Level {
  private:
    LevelTileVector tiles_;
    EntitiesVector entities_;
    std::vector<Player> players_;
    b2WorldId world_;
    LevelDimensions level_size_;
    const char* level_name_;
    LevelAssetsVector required_assets_;
    Camera camera_;
    LevelDrawInfo draw_info_;
    size_t tile_count_ = 0; // Incremented when addTile is run

    std::array<SDL_Texture*, static_cast<size_t>(asset_paths::textures::TileTypes::tile_count)>
        tile_texture_cache_{};

    uint64_t current_time_ = 0;
    uint64_t last_time_ = 0;
    float accumulator_ = 0.f;
    const float physics_step_ = 1.0f / 60.0f;
    float alpha_ = 0.f; // Value between 0.0 and 1.0 representing how far the game is between the
                        // last physics step and the next.

    void draw_tile(
        asset_paths::textures::TileTypes tile_id,
        size_t x,
        size_t y,
        WindowManager& window,
        float camera_scale
    );

    void load_level_asset(
        const LevelAsset& asset, AssetManager& asset_manager, AudioManager& audio_manager
    );
    void unload_level_asset(const LevelAsset& asset, AssetManager& asset_manager);
    void handle_previous_assets_vector(
        const LevelAssetsVector& previous_assets, AssetManager& asset_manager
    );

  public:
    Level(
        const char* level_name,
        LevelDimensions size,
        WindowManager& window,
        AssetManager& asset_manager,
        AudioManager& audio_manager,
        LevelAssetsVector required_assets,
        std::optional<const LevelAssetsVector> previous_assets = std::nullopt
    );
    ~Level();

    bool show_fan_triangulation = false;
    bool show_hitboxes = false;
    bool show_level_bounds = false;
    SDL_Color background_color = colors::sky_blue;

    void update();
    void handle_input(game_event_types::Input event);
    void draw(WindowManager& window, AssetManager& asset_manager);

    b2WorldId get_world_id() const;
    LevelDimensions get_size() const;
    Camera* get_camera();
    std::string_view get_name() const;
    size_t get_tile_count() const;
    const EntitiesVector& get_entities() const;

    void add_entity(
        b2Polygon polygon,
        b2Vec2 position,
        b2BodyDef body_def = b2DefaultBodyDef(),
        b2ShapeDef shape_def = b2DefaultShapeDef(),
        SDL_FColor hitbox_color = color_to_fcolor(colors::yellow),
        SDL_Texture* texture = nullptr,
        std::optional<b2Vec2> texture_size = std::nullopt
    );

    const LevelTileVector& get_tiles() const;
    void add_tile(asset_paths::textures::TileTypes tile_id, size_t x, size_t y);
    void remove_tile(size_t x, size_t y);

    const std::vector<Player>& get_players() const;
    void add_player(InputSource player_source, AssetManager& assets);
    void update_players(const PlayerSources& player_sources, AssetManager& assets);

    const LevelDrawInfo& drawn_last_frame() const;
    void load_required_assets(AssetManager& asset_manager, AudioManager& audio_manager);
    void unload_required_assets(AssetManager& asset_manager);

    const LevelAssetsVector& get_required_assets() const {
        return required_assets_;
    }

    Entity* get_player_entity(size_t player_index);
};

std::unique_ptr<Level> get_test_level(
    AssetManager& asset_manager,
    WindowManager& window,
    AudioManager& audio_manager,
    const LevelAssetsVector& previous_assets
);