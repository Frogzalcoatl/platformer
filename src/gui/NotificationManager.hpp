#pragma once
#include "system/WindowManager.hpp"
#include <SDL3/SDL.h>
#include <functional>
#include <string>
#include <vector>


struct Notification {
    std::string message;
    bool dismissed = false;
    Uint64 timestamp;
    std::function<void()> onClick = nullptr;
};

class NotificationManager {
  private:
    std::vector<Notification> notifications;

    void removeIndex(size_t i);

    void draw(WindowManager& window, const float uiScale);

  public:
    NotificationManager() = default;

    // Length of time notifications are shown before being removed
    unsigned int durationSeconds = 10;

    void send(std::string_view message, std::function<void()> onClick = nullptr);

    void update(WindowManager& windowManager, const float uiScale);
};