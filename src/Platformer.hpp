#pragma once
#include "AssetManager.hpp"
#include "AudioManager.hpp"
#include "InputManager.hpp"
#include "Level.hpp"
#include "UiManager.hpp"
#include "WindowManager.hpp"
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
    bool showFanTriangulation = false;

    void handleSdlEvent();
    void handleGameEvent();

  public:
    Platformer();
    void run();
    void close();
};