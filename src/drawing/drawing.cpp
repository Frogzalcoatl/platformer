#include "drawing/drawing.h"
#include <SDL3_image/SDL_image.h>
#include <array>
#include <cassert>
#include <cmath>
#include <vector>

void drawing::polygon(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    SDL_FColor color
) {
    assert(polygon.count >= 3 && polygon.count <= B2_MAX_POLYGON_VERTICES);
    SDL_Renderer* renderer = window.get_sdl_renderer();
    if (!renderer) {
        return;
    }
    std::array<SDL_FPoint, B2_MAX_POLYGON_VERTICES> points;
    float window_height = static_cast<float>(window.get_size().y);
    float camera_offset_x = static_cast<float>(camera_offset_pixels.x);
    float camera_offset_y = static_cast<float>(camera_offset_pixels.y);
    if (polygon.radius < 0.001f) {
        for (size_t i = 0; i < static_cast<size_t>(polygon.count); i++) {
            b2Vec2 pos = b2TransformPoint(transform, polygon.vertices[i]);
            pos.x = pos.x * camera_scale - camera_offset_x;
            pos.y = window_height - (pos.y * camera_scale - camera_offset_y);
            points[i] = SDL_FPoint{pos.x, pos.y};
        }
        std::array<SDL_Vertex, B2_MAX_POLYGON_VERTICES> vertices;
        for (size_t i = 0; i < static_cast<size_t>(polygon.count); i++) {
            vertices[i].color = color;
            vertices[i].position = points[i];
        }
        // Fan triangulation
        std::vector<int> indices;
        indices.reserve(static_cast<size_t>(polygon.count) * 3 - 2);
        for (int current = 2; current <= polygon.count - 1; current++) {
            indices.push_back(0);
            indices.push_back(current - 1);
            indices.push_back(current);
        }
        SDL_RenderGeometry(
            renderer,
            nullptr,
            vertices.data(),
            polygon.count,
            indices.data(),
            static_cast<int>(indices.size())
        );
        return;
    }
    // Used AI for help with radius stuff
    std::vector<SDL_FPoint> rounded_points;
    const size_t count = static_cast<size_t>(polygon.count);
    for (size_t i = 0; i < count; i++) {
        b2Vec2 v = polygon.vertices[i];
        size_t prev_index = (i + count - 1) % count;
        b2Vec2 n_in = polygon.normals[prev_index];
        b2Vec2 n_out = polygon.normals[i];
        float theta_start = atan2f(n_in.y, n_in.x);
        float theta_end = atan2f(n_out.y, n_out.x);
        float diff = theta_end - theta_start;
        if (diff < 0.f) {
            diff += 2.f * SDL_PI_F;
        }
        float radius_pixels = polygon.radius * camera_scale;
        float arc_length_pixels = radius_pixels * diff;
        int arc_segments = static_cast<int>(arc_length_pixels / 3.f);
        if (arc_segments < 3) {
            arc_segments = 3;
        }
        for (int j = 0; j <= arc_segments; j++) {
            float angle =
                theta_start + diff * (static_cast<float>(j) / static_cast<float>(arc_segments));
            b2Vec2 offset = {polygon.radius * cosf(angle), polygon.radius * sinf(angle)};
            b2Vec2 p_local = {v.x + offset.x, v.y + offset.y};
            b2Vec2 p_world = b2TransformPoint(transform, p_local);
            SDL_FPoint p_screen;
            p_screen.x = p_world.x * camera_scale - camera_offset_x;
            p_screen.y = window_height - (p_world.y * camera_scale - camera_offset_y);
            rounded_points.push_back(p_screen);
        }
    }
    size_t vertex_count = rounded_points.size();
    if (vertex_count < 3) {
        return;
    }
    std::vector<SDL_Vertex> vertices(vertex_count);
    for (size_t i = 0; i < vertex_count; i++) {
        vertices[i].color = color;
        vertices[i].position = rounded_points[i];
    }
    std::vector<int> indices;
    indices.reserve(vertex_count * 3 - 2);
    for (size_t current = 2; current < vertex_count; current++) {
        indices.push_back(0);
        indices.push_back(static_cast<int>(current - 1));
        indices.push_back(static_cast<int>(current));
    }
    SDL_RenderGeometry(
        renderer,
        nullptr,
        vertices.data(),
        static_cast<int>(vertex_count),
        indices.data(),
        static_cast<int>(indices.size())
    );
}

void drawing::polygon_borders(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    SDL_FColor color
) {
    assert(polygon.count >= 3 && polygon.count <= B2_MAX_POLYGON_VERTICES);
    SDL_Renderer* renderer = window.get_sdl_renderer();
    if (!renderer) {
        return;
    }
    float window_height = static_cast<float>(window.get_size().y);
    float camera_offset_x = static_cast<float>(camera_offset_pixels.x);
    float camera_offset_y = static_cast<float>(camera_offset_pixels.y);
    if (polygon.radius < 0.001f) {
        std::array<SDL_FPoint, B2_MAX_POLYGON_VERTICES + 1> points;
        for (size_t i = 0; i < static_cast<size_t>(polygon.count); i++) {
            b2Vec2 pos = b2TransformPoint(transform, polygon.vertices[i]);
            points[i].x = pos.x * camera_scale - camera_offset_x;
            points[i].y = window_height - (pos.y * camera_scale - camera_offset_y);
        }
        points[static_cast<size_t>(polygon.count)] = points[0];
        SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderLines(renderer, points.data(), polygon.count + 1);
        return;
    }
    // Used AI for help with the polygon radius drawing
    std::vector<SDL_FPoint> rounded_points;
    const size_t count = static_cast<size_t>(polygon.count);
    for (size_t i = 0; i < count; i++) {
        b2Vec2 v = polygon.vertices[i];
        size_t prev_index = (i + count - 1) % count;
        b2Vec2 n_in = polygon.normals[prev_index];
        b2Vec2 n_out = polygon.normals[i];
        float theta_start = atan2f(n_in.y, n_in.x);
        float theta_end = atan2f(n_out.y, n_out.x);
        float diff = theta_end - theta_start;
        if (diff < 0.f) {
            diff += 2.f * SDL_PI_F;
        }
        float radius_pixels = polygon.radius * camera_scale;
        float arc_length_pixels = radius_pixels * diff;
        int arc_segments = static_cast<int>(arc_length_pixels / 3.f);
        if (arc_segments < 3) {
            arc_segments = 3;
        }
        for (int j = 0; j <= arc_segments; j++) {
            float angle =
                theta_start + diff * (static_cast<float>(j) / static_cast<float>(arc_segments));
            b2Vec2 offset = {polygon.radius * cosf(angle), polygon.radius * sinf(angle)};
            b2Vec2 p_local = {v.x + offset.x, v.y + offset.y};
            b2Vec2 p_world = b2TransformPoint(transform, p_local);
            SDL_FPoint p_screen;
            p_screen.x = p_world.x * camera_scale - camera_offset_x;
            p_screen.y = window_height - (p_world.y * camera_scale - camera_offset_y);
            rounded_points.push_back(p_screen);
        }
    }
    if (!rounded_points.empty()) {
        rounded_points.push_back(rounded_points[0]);
    }
    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderLines(renderer, rounded_points.data(), static_cast<int>(rounded_points.size()));
}

void drawing::show_fan_triangulation(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    SDL_FColor color
) {
    assert(polygon.count >= 3 && polygon.count <= B2_MAX_POLYGON_VERTICES);
    SDL_Renderer* renderer = window.get_sdl_renderer();
    if (!renderer) {
        return;
    }
    std::array<SDL_FPoint, B2_MAX_POLYGON_VERTICES + 1> points;
    float window_height = static_cast<float>(window.get_size().y);
    float camera_offset_x = static_cast<float>(camera_offset_pixels.x);
    float camera_offset_y = static_cast<float>(camera_offset_pixels.y);
    for (size_t i = 0; i < static_cast<size_t>(polygon.count); i++) {
        b2Vec2 pos = b2TransformPoint(transform, polygon.vertices[i]);
        points[i].x = pos.x * camera_scale - camera_offset_x;
        points[i].y = window_height - (pos.y * camera_scale - camera_offset_y);
    }
    points[static_cast<size_t>(polygon.count)] = points[0];
    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderLines(renderer, points.data(), polygon.count + 1);
    for (size_t i = 2; i < static_cast<size_t>(polygon.count); i++) {
        SDL_RenderLine(renderer, points[0].x, points[0].y, points[i].x, points[i].y);
    }
}

void drawing::rectangle_borders(
    b2Vec2 min,
    b2Vec2 max,
    WindowManager& window,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    SDL_FColor color
) {
    SDL_Renderer* renderer = window.get_sdl_renderer();
    if (!renderer) {
        return;
    }
    float window_height = static_cast<float>(window.get_size().y);
    float camera_offset_x = static_cast<float>(camera_offset_pixels.x);
    float camera_offset_y = static_cast<float>(camera_offset_pixels.y);
    SDL_FPoint points[5];
    points[0].x = min.x * camera_scale - camera_offset_x;
    points[0].y = window_height - (min.y * camera_scale - camera_offset_y);
    points[1].x = max.x * camera_scale - camera_offset_x;
    points[1].y = window_height - (min.y * camera_scale - camera_offset_y);
    points[2].x = max.x * camera_scale - camera_offset_x;
    points[2].y = window_height - (max.y * camera_scale - camera_offset_y);
    points[3].x = min.x * camera_scale - camera_offset_x;
    points[3].y = window_height - (max.y * camera_scale - camera_offset_y);
    points[4] = points[0];
    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderLines(renderer, points, 5);
}

void drawing::text(
    TTF_Text* text,
    WindowManager& window,
    b2Vec2 world_position,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    float text_render_scale,
    float text_world_size_multiplier,
    SDL_FColor text_color,
    std::optional<SDL_FColor> background_color
) {
    if (!text) {
        return;
    }
    SDL_Renderer* renderer = window.get_sdl_renderer();
    if (!renderer) {
        return;
    }
    int text_width_pixels, text_height_pixels;
    if (!TTF_GetTextSize(text, &text_width_pixels, &text_height_pixels)) {
        return;
    }
    float text_scale = camera_scale / text_render_scale * text_world_size_multiplier;
    float text_height = static_cast<float>(text_height_pixels);
    float text_width = static_cast<float>(text_width_pixels);
    float window_height = static_cast<float>(window.get_size().y);
    float camera_offset_x = static_cast<float>(camera_offset_pixels.x);
    float camera_offset_y = static_cast<float>(camera_offset_pixels.y);
    SDL_FRect unscaled_text_rect;
    unscaled_text_rect.x =
        (world_position.x * camera_scale - camera_offset_x) / text_scale - (text_width / 2.f);
    unscaled_text_rect.y =
        (window_height - (world_position.y * camera_scale - camera_offset_y)) / text_scale -
        (text_height / 2.f);
    unscaled_text_rect.w = static_cast<float>(text_width_pixels);
    unscaled_text_rect.h = static_cast<float>(text_height_pixels);
    float old_render_scale_x, old_render_scale_y;
    SDL_GetRenderScale(renderer, &old_render_scale_x, &old_render_scale_y);
    SDL_SetRenderScale(renderer, text_scale, text_scale);
    if (background_color.has_value()) {
        SDL_BlendMode old_blend_mode;
        SDL_GetRenderDrawBlendMode(renderer, &old_blend_mode);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColorFloat(
            renderer,
            background_color.value().r,
            background_color.value().g,
            background_color.value().b,
            background_color.value().a
        );
        SDL_RenderFillRect(renderer, &unscaled_text_rect);
        SDL_SetRenderDrawBlendMode(renderer, old_blend_mode);
    }
    TTF_SetTextColorFloat(text, text_color.r, text_color.g, text_color.b, text_color.a);
    TTF_DrawRendererText(text, unscaled_text_rect.x, unscaled_text_rect.y);
    SDL_SetRenderScale(renderer, old_render_scale_x, old_render_scale_y);
}

void drawing::texture(
    SDL_Texture* texture,
    WindowManager& window,
    b2Vec2 world_position,
    b2Vec2 world_size,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    double sdl_angle,
    SDL_FlipMode flip
) {
    if (!texture) {
        return;
    }
    SDL_Renderer* renderer = window.get_sdl_renderer();
    if (!renderer) {
        return;
    }
    float window_height = static_cast<float>(window.get_size().y);
    float camera_offset_x = static_cast<float>(camera_offset_pixels.x);
    float camera_offset_y = static_cast<float>(camera_offset_pixels.y);
    SDL_FRect rect;
    rect.w = world_size.x * camera_scale;
    rect.h = world_size.y * camera_scale;
    rect.x = (world_position.x - world_size.x / 2.f) * camera_scale - camera_offset_x;
    rect.y =
        window_height - ((world_position.y + world_size.y / 2.f) * camera_scale - camera_offset_y);
    if (sdl_angle == 0.0 && flip == SDL_FLIP_NONE) {
        SDL_RenderTexture(renderer, texture, nullptr, &rect);
    } else {
        SDL_RenderTextureRotated(renderer, texture, nullptr, &rect, sdl_angle, nullptr, flip);
    }
}

double drawing::b2_rot_to_sdl_angle(b2Rot rotation) {
    float radians = b2Rot_GetAngle(rotation);
    return -static_cast<double>(radians) * (180.0 / SDL_PI_D);
}

b2Vec2 drawing::get_text_world_size(
    TTF_Text* text, float text_render_scale, float text_world_size_multiplier
) {
    if (!text) {
        return b2Vec2{0.f, 0.f};
    }
    int text_width_pixels, text_height_pixels;
    if (!TTF_GetTextSize(text, &text_width_pixels, &text_height_pixels)) {
        return b2Vec2{0.f, 0.f};
    }
    float world_width =
        (static_cast<float>(text_width_pixels) / text_render_scale) * text_world_size_multiplier;
    float world_height =
        (static_cast<float>(text_height_pixels) / text_render_scale) * text_world_size_multiplier;
    return b2Vec2{world_width, world_height};
}

bool drawing::should_draw_object(
    b2Vec2 object_pos_bottom_left,
    b2Vec2 object_size,
    float min_x,
    float max_x,
    float min_y,
    float max_y
) {
    float object_min_x = object_pos_bottom_left.x;
    float object_max_x = object_pos_bottom_left.x + object_size.x;
    float object_min_y = object_pos_bottom_left.y;
    float object_max_y = object_pos_bottom_left.y + object_size.y;
    if (object_min_x > max_x || object_min_y > max_y || object_max_x < min_x ||
        object_max_y < min_y) {
        return false;
    } else {
        return true;
    }
}