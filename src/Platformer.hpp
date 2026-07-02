#pragma once
#include "AssetManager.hpp"
#include "AudioManager.hpp"
#include "InputManager.hpp"
#include "Level.hpp"
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
    std::unique_ptr<Level> currentLevel;
    bool showFanTriangulation = false;

    float physicsStepHandler();

    void handleSdlEvent();
    void handleGameEvent();

  public:
    Platformer();
    void run();
    void close();
};