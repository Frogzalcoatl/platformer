#pragma once
#include "AssetManager.hpp"
#include "AudioManager.hpp"
#include "Camera.hpp"
#include "Entity.hpp"
#include "InputManager.hpp"
#include "Tile.hpp"
#include "WindowManager.hpp"
#include <box2d/box2d.h>
#include <memory>
#include <vector>

class Platformer {
  private:
    WindowManager window;
    AssetManager assets;
    AudioManager audio;
    InputManager input;
    b2WorldId world;
    bool running = false;
    std::vector<std::unique_ptr<Entity>> entities;
    std::vector<std::unique_ptr<Tile>> tiles;
    Entity* player = nullptr;
    Camera camera;

    uint64_t currentTime = 0;
    uint64_t lastTime = 0;
    float accumulator = 0.f;
    const float physicsStep = 1.0f / 60.0f;
    float physicsStepHandler();

    void handleSdlEvent();
    void handleGameEvent();
    void showDebugUi() const;

  public:
    Platformer();
    void run();
    void close();
};