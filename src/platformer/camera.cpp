#include "platformer/camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera(Entity* follow_entity, WindowManager& window)
    : window_(window), entity_to_follow(follow_entity) {
    WindowVec2 window_size = window_.get_size();
    handle_window_resize(window_size.x, window_size.y);
}

void Camera::apply_viewable_limits(b2Vec2& cam_pos) {
    const b2Vec2 window_size_world = get_size();
    if (min_viewable_x.has_value() && max_viewable_x.has_value() &&
        (max_viewable_x.value() - min_viewable_x.value() < window_size_world.x)) {
        cam_pos.x = (min_viewable_x.value() + max_viewable_x.value()) / 2.f;
    } else {
        if (min_viewable_x.has_value()) {
            const float min_cam_x = min_viewable_x.value() + window_size_world.x / 2.f;
            if (cam_pos.x < min_cam_x) {
                cam_pos.x = min_cam_x;
            }
        }
        if (max_viewable_x.has_value()) {
            const float max_cam_x = max_viewable_x.value() - window_size_world.x / 2.f;
            if (cam_pos.x > max_cam_x) {
                cam_pos.x = max_cam_x;
            }
        }
    }
    if (min_viewable_y.has_value() && max_viewable_y.has_value() &&
        (max_viewable_y.value() - min_viewable_y.value() < window_size_world.y)) {
        cam_pos.y = (min_viewable_y.value() + max_viewable_y.value()) / 2.f;
    } else {
        if (min_viewable_y.has_value()) {
            const float min_cam_y = min_viewable_y.value() + window_size_world.y / 2.f;
            if (cam_pos.y < min_cam_y) {
                cam_pos.y = min_cam_y;
            }
        }
        if (max_viewable_y.has_value()) {
            const float max_cam_y = max_viewable_y.value() - window_size_world.y / 2.f;
            if (cam_pos.y > max_cam_y) {
                cam_pos.y = max_cam_y;
            }
        }
    }
}

void Camera::update_scale_factor(int window_size_x, int window_size_y) {
    float dividend = static_cast<float>(b2MinInt(window_size_x, window_size_y));
    scale_factor_ = dividend / 20.f * scale_multiplier_;
}

void Camera::update_offset(b2Vec2 world_position) {
    b2Vec2 size = get_size();
    offset_world_.x = world_position.x - size.x / 2.f;
    offset_world_.y = world_position.y - size.y / 2.f;
}

void Camera::run(float alpha) {
    if (!entity_to_follow) {
        return;
    }
    float delta_time = window_.get_delta_time();
    scale_multiplier_ =
        scale_multiplier_ + (target_scale_multiplier_ - scale_multiplier_) *
                                (1.f - std::exp(-zoom_smoothing_speed_ * delta_time));
    WindowVec2 window_size = window_.get_size();
    float dividend = static_cast<float>(b2MinInt(window_size.x, window_size.y));
    scale_factor_ = dividend / 20.f * scale_multiplier_;
    const b2Vec2 camera_size = get_size();
    const b2Vec2 cam_pos = {
        offset_world_.x + camera_size.x / 2.f, offset_world_.y + camera_size.y / 2.f
    };
    const b2Vec2 entity_pos = entity_to_follow->get_interpolated_position(alpha);
    const b2Vec2 window_size_world = camera_size;
    const b2Vec2 entity_offset = {entity_pos.x - cam_pos.x, entity_pos.y - cam_pos.y};
    const float half_dead_zone_x = (window_size_world.x * 0.5f) * safe_area.x;
    const float half_dead_zone_y = (window_size_world.y * 0.5f) * safe_area.y;

    entity_safe_area_val_ = {
        SDL_fabsf(entity_offset.x) / (window_size_world.x * 0.5f),
        SDL_fabsf(entity_offset.y) / (window_size_world.y * 0.5f)
    };
    safe_area_size_ = {window_size_world.x * safe_area.x, window_size_world.y * safe_area.y};
    b2Vec2 new_cam_pos = cam_pos;
    if (entity_offset.x > half_dead_zone_x) {
        new_cam_pos.x = entity_pos.x - half_dead_zone_x;
    } else if (entity_offset.x < -half_dead_zone_x) {
        new_cam_pos.x = entity_pos.x + half_dead_zone_x;
    }
    if (entity_offset.y > half_dead_zone_y) {
        new_cam_pos.y = entity_pos.y - half_dead_zone_y;
    } else if (entity_offset.y < -half_dead_zone_y) {
        new_cam_pos.y = entity_pos.y + half_dead_zone_y;
    }
    apply_viewable_limits(new_cam_pos);
    update_offset(new_cam_pos);
}

b2Vec2 Camera::get_size() const {
    WindowVec2 size = window_.get_size();
    return b2Vec2{
        static_cast<float>(size.x) / scale_factor_, static_cast<float>(size.y) / scale_factor_
    };
}

void Camera::handle_window_resize(int x, int y) {
    b2Vec2 old_size = get_size();
    b2Vec2 center_world_pos;
    center_world_pos.x = offset_world_.x + old_size.x / 2.f;
    center_world_pos.y = offset_world_.y + old_size.y / 2.f;
    update_scale_factor(x, y);
    update_offset(center_world_pos);
}

WindowVec2 Camera::get_offset_pixels() const {
    return {
        static_cast<int>(SDL_roundf(offset_world_.x * scale_factor_)),
        static_cast<int>(SDL_roundf(offset_world_.y * scale_factor_))
    };
}

b2Vec2 Camera::get_offset_world() const {
    return offset_world_;
}

float Camera::get_scale_factor() const {
    return scale_factor_;
}

float Camera::get_scale_multiplier() const {
    return scale_multiplier_;
}

b2Vec2 Camera::get_safe_area_size() const {
    return safe_area_size_;
}

b2Vec2 Camera::get_entity_safe_area_value() const {
    return entity_safe_area_val_;
}

void Camera::center_on_entity(float alpha) {
    if (!entity_to_follow) {
        return;
    }
    update_offset(entity_to_follow->get_interpolated_position(alpha));
}

void Camera::increment_scale_multiplier_smooth(float amount) {
    target_scale_multiplier_ =
        std::clamp(target_scale_multiplier_ + amount, min_scale_multiplier_, max_scale_multiplier_);
}

void Camera::increment_scale_multiplier(float amount) {
    if (scale_multiplier_ + amount <= min_scale_multiplier_ ||
        scale_multiplier_ + amount >= max_scale_multiplier_) {
        return;
    }
    scale_multiplier_ += amount;
    WindowVec2 size = window_.get_size();
    update_scale_factor(size.x, size.y);
}

void Camera::reset_scale_multiplier() {
    scale_multiplier_ = 1.f;
    WindowVec2 size = window_.get_size();
    update_scale_factor(size.x, size.y);
}

b2Vec2 Camera::pixel_pos_to_world_pos(WindowVec2 pos) {
    WindowVec2 size = window_.get_size();
    float safe_scale_factor = SDL_max(scale_factor_, 0.001f);
    return b2Vec2{
        offset_world_.x + static_cast<float>(pos.x) / safe_scale_factor,
        offset_world_.y + static_cast<float>(size.y - pos.y) / safe_scale_factor
    };
}