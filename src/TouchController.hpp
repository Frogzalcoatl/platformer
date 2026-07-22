#pragma once
#include "EntityController.hpp"
#include "WindowManager.hpp"

class TouchController {
  private:
    EntityController entityController;

    bool wasUpTouched = false;
    bool wasPauseTouched = false;

    bool isLastItemTouched(const std::vector<ImVec2>& touchPositions);

  public:
    TouchController();
    TouchController(Entity& entity);

    void draw(WindowManager& window, float uiScale);
};