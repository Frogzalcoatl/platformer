#pragma once
#include "assets/AssetManager.hpp"
#include "imgui/NotificationManager.hpp"
#include "imgui/UiManager.hpp"
#include "platformer/Level.hpp"
#include "system/AudioManager.hpp"
#include "system/InputManager.hpp"
#include "system/WindowManager.hpp"
#include "user-data/SettingsManager.hpp"
#include <memory>
#include <vector>

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