#pragma once
#include "system/window_manager.h"
#include <SDL3/SDL.h>
#include <functional>
#include <string>
#include <vector>

struct Notification {
    std::string message;
    bool dismissed = false;
    Uint64 timestamp = 0;
    std::function<void()> on_click = nullptr;
};

class NotificationManager {
  private:
    std::vector<Notification> notifications_;

    void remove_index(size_t i);
    void draw(WindowManager& window, const float ui_scale);

  public:
    NotificationManager() = default;

    unsigned int duration_seconds = 10;

    void send(std::string_view message, std::function<void()> on_click = nullptr);
    void update(WindowManager& window, const float ui_scale);
};