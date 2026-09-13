#pragma once
#include "assets/asset_manager.h"
#include "gui/notification_manager.h"
#include "gui/ui_manager.h"
#include "platformer/level.h"
#include "system/audio_manager.h"
#include "system/input_manager.h"
#include "system/window_manager.h"
#include "user_data/settings_manager.h"
#include <memory>

class Platformer {
  private:
    bool running_ = false;
    WindowManager window_;
    AssetManager assets_;
    AudioManager audio_;
    InputManager input_;
    std::unique_ptr<Level> current_level_;
    SettingsManager settings_;
    UiManager ui_;
    NotificationManager notification_manager_;

    void load_settings();
    void handle_sdl_event();
    void handle_input_game_event(const game_event_types::Input& input_event);
    void handle_game_event();

  public:
    Platformer();
    void run();
};