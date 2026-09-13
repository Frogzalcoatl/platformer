#pragma once
#include "entities/entity.h"
#include "system/window_manager.h"
#include <box2d/box2d.h>
#include <optional>

class Camera {
  private:
    b2Vec2 entity_safe_area_val_ = {0.f, 0.f};
    b2Vec2 safe_area_size_ = {0.f, 0.f};
    WindowManager& window_;
    b2Vec2 offset_world_ = {0.f, 0.f};
    float scale_factor_ = 1.f;
    float scale_multiplier_ = 1.f;
    float min_scale_multiplier_ = 0.05f;
    float max_scale_multiplier_ = 2.5f;

    float target_scale_multiplier_ = 1.f;
    float zoom_smoothing_speed_ = 10.f;

    void apply_viewable_limits(b2Vec2& cam_pos);
    void update_scale_factor(int window_size_x, int window_size_y);
    void update_offset(b2Vec2 world_position);

  public:
    Camera(Entity* follow_entity, WindowManager& window);

    void run(float alpha);

    b2Vec2 get_size() const;
    void handle_window_resize(int x, int y);

    WindowVec2 get_offset_pixels() const;
    b2Vec2 get_offset_world() const;

    float get_scale_factor() const;
    float get_scale_multiplier() const;

    b2Vec2 get_safe_area_size() const;
    b2Vec2 get_entity_safe_area_value() const;

    void center_on_entity(float alpha);
    void increment_scale_multiplier(float amount);
    void increment_scale_multiplier_smooth(float amount);
    void reset_scale_multiplier();

    b2Vec2 pixel_pos_to_world_pos(WindowVec2 pos);

    b2Vec2 safe_area = {0.15f, 0.15f};
    Entity* entity_to_follow = nullptr;

    std::optional<float> min_viewable_y = 0.f;
    std::optional<float> max_viewable_y;
    std::optional<float> min_viewable_x = 0.f;
    std::optional<float> max_viewable_x;
};