#include "platformer/level.h"
#include "drawing/drawing.h"
#include <algorithm>

using namespace asset_paths;

Level::Level(
    const char* level_name,
    LevelDimensions size,
    WindowManager& window,
    AssetManager& asset_manager,
    AudioManager& audio_manager,
    LevelAssetsVector required_assets,
    std::optional<const LevelAssetsVector> previous_assets_opt
)
    : level_size_(size), level_name_(level_name), required_assets_(required_assets),
      camera_(nullptr, window) {
    if (previous_assets_opt.has_value()) {
        handle_previous_assets_vector(previous_assets_opt.value(), asset_manager);
    }
    load_required_assets(asset_manager, audio_manager);
    b2WorldDef world_def = b2DefaultWorldDef();
    world_def.gravity = {0.0f, -60.f};
    world_ = b2CreateWorld(&world_def);
    SDL_Log("Created box2d world for level \"%s\"", level_name);
    tiles_.resize(size.width);
    for (auto& column : tiles_) {
        column.resize(size.height);
    }
    for (size_t i = 1; i < static_cast<size_t>(textures::TileTypes::tile_count); i++) {
        std::string_view path = textures::tile_paths[i];
        if (!path.empty()) {
            tile_texture_cache_[i] = asset_manager.load_texture(path);
        }
    }
}

Level::~Level() {
    entities_.clear();
    players_.clear();
    b2DestroyWorld(world_);
    SDL_Log("Destroyed box2d world for level \"%s\"", level_name_);
}

void Level::load_level_asset(
    const LevelAsset& asset, AssetManager& asset_manager, AudioManager& audio_manager
) {
    switch (asset.type) {
    case AssetTypes::audio: {
        MIX_Mixer* mixer_device = audio_manager.get_mixer_device();
        if (mixer_device) {
            asset_manager.load_audio(
                asset.relative_path, mixer_device, asset.should_be_predecoded_audio
            );
        }
    } break;
    case AssetTypes::font_sdl: {
        if (asset.font_info.has_value()) {
            const FontInfo& font_info = asset.font_info.value();
            asset_manager.load_sdl_font(asset.relative_path, font_info.pt_size, font_info.style);
        } else {
            SDL_LogWarn(
                SDL_LOG_CATEGORY_APPLICATION,
                "Unable to load font \"%.*s\": No font info provided.",
                static_cast<int>(asset.relative_path.length()),
                asset.relative_path.data()
            );
        }
    } break;
    case AssetTypes::texture: {
        asset_manager.load_texture(asset.relative_path);
    } break;
    }
}

void Level::unload_level_asset(const LevelAsset& asset, AssetManager& asset_manager) {
    switch (asset.type) {
    case AssetTypes::audio: {
        asset_manager.unload_audio(asset.relative_path, asset.should_be_predecoded_audio);
    } break;
    case AssetTypes::font_sdl: {
        if (asset.font_info.has_value()) {
            const FontInfo& font_info = asset.font_info.value();
            asset_manager.unload_sdl_font(asset.relative_path, font_info.pt_size, font_info.style);
        } else {
            SDL_LogWarn(
                SDL_LOG_CATEGORY_APPLICATION,
                "Unable to unload font \"%.*s\": No font info provided.",
                static_cast<int>(asset.relative_path.length()),
                asset.relative_path.data()
            );
        }
    } break;
    case AssetTypes::texture: {
        asset_manager.unload_texture(asset.relative_path);
    } break;
    }
}

void Level::load_required_assets(AssetManager& asset_manager, AudioManager& audio_manager) {
    for (const auto& asset : required_assets_) {
        load_level_asset(asset, asset_manager, audio_manager);
    }
}

void Level::unload_required_assets(AssetManager& asset_manager) {
    for (const auto& asset : required_assets_) {
        unload_level_asset(asset, asset_manager);
    }
}

Entity* Level::get_player_entity(size_t player_index) {
    if (player_index >= players_.size()) {
        return nullptr;
    }
    Player& player = players_[player_index];
    if (!player.controller) {
        return nullptr;
    }
    return player.controller->get_entity();
}

void Level::handle_previous_assets_vector(
    const LevelAssetsVector& previous_assets, AssetManager& asset_manager
) {
    LevelAssetsVector assets_to_unload;
    for (const auto& asset : previous_assets) {
        bool not_in_required_assets =
            std::find(required_assets_.begin(), required_assets_.end(), asset) ==
            required_assets_.end();
        if (not_in_required_assets) {
            assets_to_unload.push_back(asset);
        }
    }
    for (const auto& asset : assets_to_unload) {
        unload_level_asset(asset, asset_manager);
    }
}

void Level::draw_tile(
    textures::TileTypes tile_id, size_t x, size_t y, WindowManager& window, float camera_scale
) {
    assert(tile_id < textures::TileTypes::tile_count);
    size_t tile_index = static_cast<size_t>(tile_id);
    if (tile_index >= tile_texture_cache_.size()) {
        return;
    }
    SDL_Texture* texture = tile_texture_cache_[tile_index];
    if (!texture) {
        return;
    }
    drawing::texture(
        texture,
        window,
        b2Vec2{static_cast<float>(x + 0.5f), static_cast<float>(y + 0.5f)},
        b2Vec2{1.f, 1.f},
        camera_scale,
        camera_.get_offset_pixels()
    );
}

void Level::update() {
    current_time_ = SDL_GetTicks();
    float delta_time = static_cast<float>(current_time_ - last_time_) / 1000.0f;
    last_time_ = current_time_;
    if (delta_time > 0.1f) {
        delta_time = 0.1f;
    }
    accumulator_ += delta_time;
    while (accumulator_ >= physics_step_) {
        for (const auto& entity : entities_) {
            if (entity) {
                entity->save_previous_state();
            }
        }
        for (auto& player : players_) {
            if (player.controller) {
                player.controller->update();
            }
        }
        b2World_Step(world_, physics_step_, 4);
        accumulator_ -= physics_step_;
    }
    alpha_ = accumulator_ / physics_step_;
}

void Level::handle_input(game_event_types::Input event) {
    for (auto& player : players_) {
        if (!player.controller) {
            continue;
        }
        if (player.source == event.source_info) {
            player.controller->handle_input(event, &camera_, alpha_);
            return;
        }
    }
}

void Level::draw(WindowManager& window, AssetManager& asset_manager) {
    camera_.run(alpha_);
    if (tiles_.empty()) {
        return;
    }
    const float camera_scale = camera_.get_scale_factor();
    const WindowVec2 camera_offset_pixels = camera_.get_offset_pixels();
    const b2Vec2 camera_size_world = camera_.get_size();
    const b2Vec2 camera_offset_world = camera_.get_offset_world();
    const size_t min_x = static_cast<size_t>(SDL_max(SDL_floorf(camera_offset_world.x), 0.f));
    const size_t max_x = static_cast<size_t>(SDL_min(
        SDL_ceilf(camera_offset_world.x + camera_size_world.x),
        static_cast<float>(level_size_.width) - 1.f
    ));
    const size_t min_y = static_cast<size_t>(SDL_max(SDL_floorf(camera_offset_world.y), 0.f));
    const size_t max_y = static_cast<size_t>(SDL_min(
        SDL_ceilf(camera_offset_world.y + camera_size_world.y),
        static_cast<float>(level_size_.height) - 1.f
    ));
    draw_info_ = LevelDrawInfo{};
    for (size_t x = min_x; x <= max_x; x++) {
        for (size_t y = min_y; y <= max_y; y++) {
            if (tiles_[x][y] != textures::TileTypes::air) {
                draw_tile(tiles_[x][y], x, y, window, camera_scale);
                draw_info_.tiles++;
            }
        }
    }
    float min_x_float = static_cast<float>(min_x);
    float max_x_float = static_cast<float>(max_x);
    float min_y_float = static_cast<float>(min_y);
    float max_y_float = static_cast<float>(max_y);
    for (const auto& entity : entities_) {
        bool did_draw_entity = false;
        if (!entity) {
            continue;
        }
        b2Transform transform;
        transform.p = entity->get_interpolated_position(alpha_);
        transform.q = entity->get_interpolated_rotation(alpha_);
        b2AABB entity_aabb = b2ComputePolygonAABB(&entity->get_polygon(), transform);
        b2Vec2 entity_size;
        entity_size.x = entity_aabb.upperBound.x - entity_aabb.lowerBound.x;
        entity_size.y = entity_aabb.upperBound.y - entity_aabb.lowerBound.y;
        if (!drawing::should_draw_object(
                entity_aabb.lowerBound,
                entity_size,
                min_x_float,
                max_x_float,
                min_y_float,
                max_y_float
            )) {
            b2Vec2 nametag_pos = entity->get_nametag_world_pos(alpha_);
            b2Vec2 nametag_size = entity->get_nametag_world_size(
                asset_manager.text_render_scale, asset_manager.text_world_size_multiplier
            );
            b2Vec2 pos_bottom_left{
                nametag_pos.x - nametag_size.x / 2.f, nametag_pos.y - nametag_size.y / 2.f
            };
            if (drawing::should_draw_object(
                    pos_bottom_left,
                    nametag_size,
                    min_x_float,
                    max_x_float,
                    min_y_float,
                    max_y_float
                )) {
                entity->draw_nametag(
                    window, alpha_, camera_scale, camera_offset_pixels, asset_manager
                );
            }
            continue;
        }
        if (entity->draw(window, alpha_, camera_scale, camera_offset_pixels, asset_manager)) {
            did_draw_entity = true;
        }
        if (show_fan_triangulation) {
            drawing::show_fan_triangulation(
                entity->get_polygon(), window, transform, camera_scale, camera_offset_pixels
            );
            did_draw_entity = true;
        }
        if (show_hitboxes) {
            entity->draw_hitbox(window, alpha_, camera_scale, camera_offset_pixels);
            did_draw_entity = true;
        }
        if (did_draw_entity) {
            draw_info_.entities++;
        }
    }
    if (show_level_bounds) {
        LevelDimensions bounds = get_size();
        drawing::rectangle_borders(
            b2Vec2{0.f, 0.f},
            b2Vec2{static_cast<float>(bounds.width), static_cast<float>(bounds.height)},
            window,
            camera_scale,
            camera_offset_pixels,
            color_to_fcolor(colors::blue)
        );
    }
}

b2WorldId Level::get_world_id() const {
    return world_;
}

LevelDimensions Level::get_size() const {
    size_t width = tiles_.size();
    size_t height = 0;
    if (!tiles_.empty()) {
        height = tiles_[0].size();
    }
    return LevelDimensions{width, height};
}

Camera* Level::get_camera() {
    return &camera_;
}

std::string_view Level::get_name() const {
    return level_name_;
}

size_t Level::get_tile_count() const {
    return tile_count_;
}

const EntitiesVector& Level::get_entities() const {
    return entities_;
}

void Level::add_entity(
    b2Polygon polygon,
    b2Vec2 position,
    b2BodyDef body_def,
    b2ShapeDef shape_def,
    SDL_FColor hitbox_color,
    SDL_Texture* texture,
    std::optional<b2Vec2> texture_size
) {
    auto entity = std::make_unique<Entity>(
        world_, polygon, position, body_def, shape_def, hitbox_color, texture, texture_size
    );
    entities_.push_back(std::move(entity));
    b2World_Step(world_, physics_step_, 4);
}

const LevelTileVector& Level::get_tiles() const {
    return tiles_;
}

void Level::add_tile(textures::TileTypes tile_id, size_t x, size_t y) {
    assert(tile_id < textures::TileTypes::tile_count);
    if (tile_id >= textures::TileTypes::tile_count) {
        return;
    }
    std::string_view relative_path_str = textures::tile_paths[static_cast<size_t>(tile_id)];
    if (x >= level_size_.width || y >= level_size_.height) {
        std::filesystem::path relative_path = relative_path_str;
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Invalid tile %s position (%zu, %zu). Level size is (%zu, %zu)",
            relative_path.filename().string().c_str(),
            x,
            y,
            level_size_.width,
            level_size_.height
        );
        return;
    }
    tiles_[x][y] = tile_id;
    if (tile_id != textures::TileTypes::air) {
        tile_count_++;
    }
    if (tile_id == textures::TileTypes::air && tile_count_ > 0) {
        tile_count_--;
    }
}

void Level::remove_tile(size_t x, size_t y) {
    if (x >= level_size_.width || y >= level_size_.height) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to remove tile at position (%zu, %zu). Level size is (%zu, %zu)",
            x,
            y,
            level_size_.width,
            level_size_.height
        );
        return;
    }
    tiles_[x][y] = textures::TileTypes::air;
}

const std::vector<Player>& Level::get_players() const {
    return players_;
}

void Level::add_player(InputSource player_source, AssetManager& assets) {
    b2BodyDef player_body_def = b2DefaultBodyDef();
    player_body_def.fixedRotation = true;
    player_body_def.type = b2_dynamicBody;
    b2ShapeDef player_shape_def = b2DefaultShapeDef();
    player_shape_def.material.friction = 0.f;
    player_shape_def.density = 4.f;
    std::unique_ptr<Entity> player_entity = std::make_unique<Entity>(
        world_,
        b2MakeRoundedBox(0.35f, 0.85f, 0.15f),
        b2Vec2{10.f, 4.f},
        player_body_def,
        player_shape_def,
        color_to_fcolor(colors::yellow),
        assets.load_texture(textures::player),
        b2Vec2{1.f, 2.f}
    );
    const size_t current_player_count = players_.size();
    if (current_player_count == 0) {
        camera_.entity_to_follow = player_entity.get();
    }
    std::string nametag = "Player " + std::to_string(current_player_count + 1);
    player_entity->set_nametag(nametag, assets);
    std::unique_ptr<EntityController> entity_controller =
        std::make_unique<EntityController>(*player_entity);
    entity_controller->spawn_point = b2Vec2{4.f, 4.f};
    players_.push_back(Player{player_source, std::move(entity_controller)});
    entities_.push_back(std::move(player_entity));
    b2World_Step(world_, physics_step_, 4);
    SDL_Log("Added new player \"%s\" to level \"%s\"", nametag.c_str(), level_name_);
}

void Level::update_players(const PlayerSources& player_sources, AssetManager& assets) {
    auto player_it = players_.begin();
    while (player_it != players_.end()) {
        bool still_active =
            std::find(player_sources.begin(), player_sources.end(), player_it->source) !=
            player_sources.end();
        if (still_active) {
            player_it++;
        } else {
            Entity* entity_ptr = player_it->controller->get_entity();
            if (camera_.entity_to_follow == entity_ptr) {
                // Placeholder logic until camera following is improved
                camera_.entity_to_follow = nullptr;
            }
            std::string nametag = entity_ptr ? entity_ptr->get_nametag_str() : "";
            if (entity_ptr) {
                auto entity_it = std::find_if(
                    entities_.begin(), entities_.end(), [entity_ptr](const auto& entity) {
                        return entity.get() == entity_ptr;
                    }
                );
                if (entity_it != entities_.end()) {
                    entities_.erase(entity_it);
                }
            }
            player_it = players_.erase(player_it);
            SDL_Log("Removed player \"%s\"", nametag.c_str());
        }
    }
    // Update nametags in case indices were shifted.
    for (size_t i = 0; i < players_.size(); i++) {
        Entity* player_entity = players_[i].controller->get_entity();
        if (player_entity) {
            std::string nametag = "Player " + std::to_string(i + 1);
            player_entity->set_nametag(nametag, assets);
        }
    }
    // Add players for new player sources
    for (const auto player_source_opt : player_sources) {
        if (!player_source_opt.has_value()) {
            continue;
        }
        const InputSource& player_source = player_source_opt.value();
        // In lambda functions [] is the capture clause.
        // [&] means the lambda can access any variable in its outer scope.
        bool already_exists =
            std::any_of(players_.begin(), players_.end(), [&player_source](const Player& p) {
                return player_source == p.source;
            });
        if (!already_exists) {
            add_player(player_source, assets);
        }
    }
    if (!players_.empty()) {
        camera_.entity_to_follow = players_[0].controller->get_entity();
    }
}

const LevelDrawInfo& Level::drawn_last_frame() const {
    return draw_info_;
}

std::unique_ptr<Level> get_test_level(
    AssetManager& asset_manager,
    WindowManager& window,
    AudioManager& audio_manager,
    const LevelAssetsVector& previous_assets
) {
    std::unique_ptr<Level> level = std::make_unique<Level>(
        "Test",
        LevelDimensions{100, 40},
        window,
        asset_manager,
        audio_manager,
        level_assets::template_assets,
        previous_assets
    );
    level->show_level_bounds = true;
    const int ground_width = 50;
    const int ground_height = 2;
    const int wall_height = 20;
    const int wall_pos_left = 0;
    const int wall_pos_right = ground_width;
    level->add_entity(
        b2MakeBox(static_cast<float>(ground_width) / 2.f, static_cast<float>(ground_height) / 2.f),
        b2Vec2{static_cast<float>(ground_width) / 2.f, 1.f}
    );
    level->add_entity(
        b2MakeBox(0.5f, static_cast<float>(wall_height) / 2.f),
        b2Vec2{wall_pos_left + 0.5f, static_cast<float>(wall_height) / 2.f}
    );
    level->add_entity(
        b2MakeBox(0.5f, static_cast<float>(wall_height) / 2.f),
        b2Vec2{wall_pos_right + 0.5f, static_cast<float>(wall_height) / 2.f}
    );
    b2BodyDef dynamic_body_def = b2DefaultBodyDef();
    dynamic_body_def.type = b2_dynamicBody;
    level->add_entity(
        b2MakeBox(0.5f, 2.f),
        b2Vec2{28.f, 4.f},
        dynamic_body_def,
        b2DefaultShapeDef(),
        color_to_fcolor(colors::yellow),
        asset_manager.load_texture(textures::log),
        b2Vec2{1.f, 4.f}
    );
    level->add_entity(
        b2MakeBox(0.5f, 1.f),
        b2Vec2{8.f, 3.f},
        dynamic_body_def,
        b2DefaultShapeDef(),
        color_to_fcolor(colors::yellow),
        asset_manager.load_texture(textures::log),
        b2Vec2{1.f, 2.f}
    );
    for (size_t i = 1; i < ground_width; i++) {
        level->add_tile(textures::TileTypes::dirt, i, 0);
        level->add_tile(textures::TileTypes::grass, i, 1);
    }
    for (size_t i = 0; i < wall_height; i++) {
        level->add_tile(textures::TileTypes::stone, wall_pos_left, i);
        level->add_tile(textures::TileTypes::stone, wall_pos_right, i);
    }
    return level;
}