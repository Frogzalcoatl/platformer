#pragma once
#include "assets/asset_manager.h"
#include "assets/colors.h"
#include "system/window_manager.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <optional>
#include <string>

enum class EntityMovement : uint8_t {
    up,
    down,
    left,
    right,
    entity_movement_count
};

class Entity {
  private:
    b2BodyId body_id_;
    b2Polygon polygon_;
    b2Vec2 position_last_real_frame_;
    float angle_last_real_frame_;
    SDL_Texture* texture_;
    std::optional<b2Vec2> texture_size_;
    UniqueText nametag_;

  public:
    Entity(
        b2WorldId world,
        b2Polygon polygon,
        b2Vec2 position,
        b2BodyDef body_def = b2DefaultBodyDef(),
        b2ShapeDef shape_def = b2DefaultShapeDef(),
        SDL_FColor hitbox_color = color_to_fcolor(colors::yellow),
        SDL_Texture* texture = nullptr,
        std::optional<b2Vec2> texture_size = std::nullopt
    );
    ~Entity();

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    SDL_FColor hitbox_color;

    const b2BodyId& get_body_id() const;
    const b2Polygon& get_polygon() const;
    b2Vec2 get_position() const;
    b2Vec2 get_interpolated_position(float alpha) const;
    b2Rot get_interpolated_rotation(float alpha) const;

    void save_previous_state();

    bool draw(
        WindowManager& window,
        float alpha,
        float camera_scale,
        WindowVec2 camera_offset_pixels,
        AssetManager& assets
    ) const;

    bool draw_nametag(
        WindowManager& window,
        float alpha,
        float camera_scale,
        WindowVec2 camera_offset_pixels,
        AssetManager& assets
    ) const;

    void draw_hitbox(
        WindowManager& window, float alpha, float camera_scale, WindowVec2 camera_offset_pixels
    ) const;

    void teleport(b2Vec2 location);

    void set_nametag(std::string_view text, AssetManager& assets);

    b2Vec2 get_nametag_world_size(float text_render_scale, float text_world_size_multiplier) const;

    b2Vec2 get_nametag_world_pos(float alpha) const;

    std::string get_nametag_str() const;
};