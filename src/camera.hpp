#pragma once
#include "entity.hpp"
#include "windowManager.hpp"
#include <box2d/box2d.h>
#include <optional>

class Camera {
  private:
    b2Vec2 entitySafeAreaVal;
    b2Vec2 safeAreaSize;
    WindowManager& window;

    b2Vec2 applyViewableLimits(b2Vec2 camPos);

  public:
    Camera(Entity* followEntity, WindowManager& window);
    void run(void);
    b2Vec2 getEntitySafeAreaValue(void);
    b2Vec2 getSafeAreaSize(void);
    // void centerOnEntity(void);
    b2Vec2 safeArea = {0.5f, 0.5f};
    Entity* entityToFollow;
    std::optional<float> minViewableY;
    std::optional<float> maxViewableY;
    std::optional<float> minViewableX;
    std::optional<float> maxViewableX;
};