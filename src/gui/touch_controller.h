#pragma once
#include "platformer/entity_controller.h"
#include "system/window_manager.h"
#include <vector>

struct TouchButtonRect {
    ImVec2 min;
    ImVec2 max;
};

class TouchController {
  private:
    EntityController entity_controller_;

    bool was_up_touched_ = false;
    bool was_pause_touched_ = false;
    int free_finger_count_ = 0;

    bool is_last_item_touched(const std::vector<ImVec2>& touch_positions);

  public:
    TouchController();
    TouchController(Entity& entity);

    void draw(WindowManager& window, float ui_scale);

    int get_free_finger_count() const {
        return free_finger_count_;
    }
};