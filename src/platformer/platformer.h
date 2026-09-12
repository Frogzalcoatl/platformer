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
    bool running = false;
    WindowManager window;
    AssetManager assets;
    AudioManager audio;
    InputManager input;
    std::unique_ptr<Level> currentLevel;
    SettingsManager settings;
    UiManager ui;
    NotificationManager notificationManager;

    void loadSettings();

    void handleSdlEvent();

    void handleInputGameEvent(const GameEventTypes::Input& inputEvent);

    void handleGameEvent();

  public:
    Platformer();
    void run();
};