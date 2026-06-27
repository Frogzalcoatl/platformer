#pragma once
#include "camera.hpp"
#include "entity.hpp"
#include "windowManager.hpp"
#include <box2d/box2d.h>
#include <memory>
#include <vector>

class Platformer {
  private:
    WindowManager window;
    b2WorldId world;
    bool running;
    std::vector<std::unique_ptr<Entity>> entities;
    Entity* player;
    Camera camera;

    uint64_t currentTime = 0;
    uint64_t lastTime = 0;
    float accumulator = 0.f;
    const float physicsStep = 1.0f / 60.0f;
    void physicsStepHandler();

    void handleSdlEvent();
    void handleGameEvent();
    void drawDebugUi() const;

  public:
    Platformer();
    void run();
    void close();
};