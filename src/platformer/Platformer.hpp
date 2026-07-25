#pragma once
#include "assets/AssetManager.hpp"
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
    UiManager ui;
    std::unique_ptr<Level> currentLevel;
    SettingsManager settings;

    void loadSettings(bool readFromDisk);

    void handleSdlEvent();

    void handleInputGameEvent(const GameEventTypes::Input& inputEvent);

    void handleGameEvent();

  public:
    Platformer();
    void run();
};