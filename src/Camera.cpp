#include "Camera.hpp"

Camera::Camera(Entity* followEntity, WindowManager& window)
    : window(window), entityToFollow(followEntity) {
    WindowVec2 windowSize = window.getSize();
    handleWindowResize(windowSize.x, windowSize.y);
}

b2Vec2 Camera::getEntitySafeAreaValue() const {
    return entitySafeAreaVal;
}

b2Vec2 Camera::getSafeAreaSize() const {
    return safeAreaSize;
}

void Camera::updateScaleFactor(int windowSizeX, int windowSizeY) {
    int dividend = b2MinInt(windowSizeX, windowSizeY);
    scaleFactor = dividend / 20.f * scaleMultiplier;
}

void Camera::updateOffset(b2Vec2 worldPosition) {
    b2Vec2 size = getSize();
    offsetWorld = b2Vec2{worldPosition.x - size.x / 2.f, worldPosition.y - size.y / 2.f};
}

void Camera::handleWindowResize(int x, int y) {
    b2Vec2 oldSize = getSize();
    b2Vec2 center = {offsetWorld.x + oldSize.x / 2.f, offsetWorld.y + oldSize.y / 2.f};
    updateScaleFactor(x, y);
    updateOffset(center);
}

b2Vec2 Camera::getSize() const {
    WindowVec2 size = window.getSize();
    return b2Vec2{size.x / scaleFactor, size.y / scaleFactor};
}

WindowVec2 Camera::getOffsetPixels() const {
    return {
        static_cast<int>(SDL_roundf(offsetWorld.x * scaleFactor)),
        static_cast<int>(SDL_roundf(offsetWorld.y * scaleFactor))
    };
}

b2Vec2 Camera::getOffsetWorld() const {
    return offsetWorld;
}

float Camera::getScaleFactor() const {
    return scaleFactor;
}

void Camera::incrementScaleMultiplierBy(float amount) {
    if (scaleMultiplier + amount <= 0) {
        return;
    }
    scaleMultiplier += amount;
    WindowVec2 size = window.getSize();
    updateScaleFactor(size.x, size.y);
}

void Camera::resetScaleMultiplier() {
    scaleMultiplier = 1.f;
    WindowVec2 size = window.getSize();
    updateScaleFactor(size.x, size.y);
}

b2Vec2 Camera::pixelPosToWorldPos(WindowVec2 pos) {
    WindowVec2 size = window.getSize();
    float safeScaleFactor = SDL_max(scaleFactor, 0.001f);
    return b2Vec2{
        offsetWorld.x + static_cast<float>(pos.x) / safeScaleFactor,
        offsetWorld.y + static_cast<float>(size.y - pos.y) / safeScaleFactor
    };
}

void Camera::run(float alpha) {
    if (!entityToFollow) {
        return;
    }
    const b2Vec2 size = getSize();
    const b2Vec2 camPos = {offsetWorld.x + size.x / 2.f, offsetWorld.y + size.y / 2.f};
    const b2Vec2 entityPos = entityToFollow->getInterpolatedPosition(alpha);
    const b2Vec2 windowSizeWorld = size;
    const b2Vec2 entityOffset = {entityPos.x - camPos.x, entityPos.y - camPos.y};
    const float halfDeadZoneX = (windowSizeWorld.x * 0.5f) * safeArea.x;
    const float halfDeadZoneY = (windowSizeWorld.y * 0.5f) * safeArea.y;
    entitySafeAreaVal = {
        SDL_fabsf(entityOffset.x) / (windowSizeWorld.x * 0.5f),
        SDL_fabsf(entityOffset.y) / (windowSizeWorld.y * 0.5f)
    };
    safeAreaSize = {windowSizeWorld.x * safeArea.x, windowSizeWorld.y * safeArea.y};
    b2Vec2 newCamPos = camPos;
    if (entityOffset.x > halfDeadZoneX) {
        newCamPos.x = entityPos.x - halfDeadZoneX;
    } else if (entityOffset.x < -halfDeadZoneX) {
        newCamPos.x = entityPos.x + halfDeadZoneX;
    }
    if (entityOffset.y > halfDeadZoneY) {
        newCamPos.y = entityPos.y - halfDeadZoneY;
    } else if (entityOffset.y < -halfDeadZoneY) {
        newCamPos.y = entityPos.y + halfDeadZoneY;
    }
    applyViewableLimits(newCamPos);
    updateOffset(newCamPos);
}

void Camera::applyViewableLimits(b2Vec2& camPos) {
    const b2Vec2 windowSizeWorld = getSize();
    if (minViewableX.has_value() && maxViewableX.has_value() &&
        (maxViewableX.value() - minViewableX.value() < windowSizeWorld.x)) {
        camPos.x = (minViewableX.value() + maxViewableX.value()) / 2.f;
    } else {
        if (minViewableX.has_value()) {
            const float minCamX = minViewableX.value() + windowSizeWorld.x / 2.f;
            if (camPos.x < minCamX) {
                camPos.x = minCamX;
            }
        }
        if (maxViewableX.has_value()) {
            const float maxCamX = maxViewableX.value() - windowSizeWorld.x / 2.f;
            if (camPos.x > maxCamX) {
                camPos.x = maxCamX;
            }
        }
    }
    if (minViewableY.has_value() && maxViewableY.has_value() &&
        (maxViewableY.value() - minViewableY.value() < windowSizeWorld.y)) {
        camPos.y = (minViewableY.value() + maxViewableY.value()) / 2.f;
    } else {
        if (minViewableY.has_value()) {
            const float minCamY = minViewableY.value() + windowSizeWorld.y / 2.f;
            if (camPos.y < minCamY) {
                camPos.y = minCamY;
            }
        }
        if (maxViewableY.has_value()) {
            const float maxCamY = maxViewableY.value() - windowSizeWorld.y / 2.f;
            if (camPos.y > maxCamY) {
                camPos.y = maxCamY;
            }
        }
    }
}

void Camera::centerOnEntity() {
    if (!entityToFollow) {
        return;
    }
    updateOffset(entityToFollow->getPosition());
}

void Camera::centerOnEntity(float alpha) {
    if (!entityToFollow) {
        return;
    }
    updateOffset(entityToFollow->getInterpolatedPosition(alpha));
}
