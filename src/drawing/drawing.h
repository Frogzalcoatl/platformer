#pragma once
#include "assets/colors.h"
#include "system/window_manager.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <box2d/box2d.h>
#include <optional>

namespace drawing {
void polygon(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    SDL_FColor color = color_to_fcolor(colors::white)
);

void polygon_borders(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    SDL_FColor color = color_to_fcolor(colors::black)
);

void show_fan_triangulation(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    SDL_FColor color = color_to_fcolor(colors::red)
);

void rectangle_borders(
    b2Vec2 min,
    b2Vec2 max,
    WindowManager& window,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    SDL_FColor color
);

void text(
    TTF_Text* text,
    WindowManager& window,
    b2Vec2 world_position,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    // Next two params are consts from AssetManager
    float text_render_scale,
    float text_world_size_multiplier,
    SDL_FColor text_color = color_to_fcolor(colors::white),
    std::optional<SDL_FColor> background_color = SDL_FColor{0.f, 0.f, 0.f, 0.5f}
);

void texture(
    SDL_Texture* texture,
    WindowManager& window,
    b2Vec2 world_position,
    b2Vec2 world_size,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    double sdl_angle = 0.0,
    SDL_FlipMode flip = SDL_FLIP_NONE
);

double b2_rot_to_sdl_angle(b2Rot rotation);
b2Vec2
get_text_world_size(TTF_Text* text, float text_render_scale, float text_world_size_multiplier);
bool should_draw_object(
    b2Vec2 object_pos_bottom_left,
    b2Vec2 object_size,
    float min_x,
    float max_x,
    float min_y,
    float max_y
);
}