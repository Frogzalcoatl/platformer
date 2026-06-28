#pragma once
#include "entity.hpp"
#include "windowManager.hpp"
#include <box2d/box2d.h>
#include <optional>

class Camera {
  private:
    b2Vec2 entitySafeAreaVal = {0.f, 0.f};
    b2Vec2 safeAreaSize = {0.f, 0.f};
    WindowManager& window;

    void applyViewableLimits(b2Vec2& camPos);

  public:
    Camera(Entity* followEntity, WindowManager& window);
    void run();
    b2Vec2 getEntitySafeAreaValue() const;
    b2Vec2 getSafeAreaSize() const;
    void centerOnEntity();
    b2Vec2 safeArea = {0.5f, 0.5f};
    Entity* entityToFollow;
    std::optional<float> minViewableY;
    std::optional<float> maxViewableY;
    std::optional<float> minViewableX;
    std::optional<float> maxViewableX;
};