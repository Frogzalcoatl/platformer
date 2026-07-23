#pragma once
#include "EntityController.hpp"
#include "WindowManager.hpp"

struct TouchButtonRect {
    ImVec2 min;
    ImVec2 max;
};

class TouchController {
  private:
    EntityController entityController;

    bool wasUpTouched = false;
    bool wasPauseTouched = false;
    int freeFingerCount = 0;

    bool isLastItemTouched(const std::vector<ImVec2>& touchPositions);

  public:
    TouchController();
    TouchController(Entity& entity);

    void draw(WindowManager& window, float uiScale);

    int getFreeFingerCount() const {
        return freeFingerCount;
    }
};