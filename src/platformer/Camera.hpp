#pragma once
#include "entities/Entity.hpp"
#include "system/WindowManager.hpp"
#include <box2d/box2d.h>
#include <optional>

class Camera {
  private:
    b2Vec2 entitySafeAreaVal = {0.f, 0.f};
    b2Vec2 safeAreaSize = {0.f, 0.f};
    WindowManager& window;
    b2Vec2 offsetWorld = {0.f, 0.f};
    float scaleFactor = 1.f;
    float scaleMultiplier = 1.f;

    void applyViewableLimits(b2Vec2& camPos);

    void updateScaleFactor(int windowSizeX, int windowSizeY);

    void updateOffset(b2Vec2 worldPosition);

  public:
    Camera(Entity* followEntity, WindowManager& window);

    void run(float alpha);

    b2Vec2 getSize() const;

    void handleWindowResize(int x, int y);

    WindowVec2 getOffsetPixels() const;

    b2Vec2 getOffsetWorld() const;

    float getScaleFactor() const;

    b2Vec2 getSafeAreaSize() const;

    b2Vec2 getEntitySafeAreaValue() const;

    void centerOnEntity(float alpha);

    void incrementScaleMultiplierBy(float amount);

    void resetScaleMultiplier();

    b2Vec2 pixelPosToWorldPos(WindowVec2 pos);

    b2Vec2 safeArea = {0.15f, 0.15f};

    Entity* entityToFollow;

    std::optional<float> minViewableY = 0.f;
    std::optional<float> maxViewableY;
    std::optional<float> minViewableX = 0.f;
    std::optional<float> maxViewableX;
};