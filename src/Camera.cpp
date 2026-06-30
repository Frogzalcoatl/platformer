#include "Camera.hpp"

Camera::Camera(Entity* followEntity, WindowManager& window) : window(window) {
    entityToFollow = followEntity;
}

b2Vec2 Camera::getEntitySafeAreaValue() const {
    return entitySafeAreaVal;
}

b2Vec2 Camera::getSafeAreaSize() const {
    return safeAreaSize;
}

void Camera::run() {
    if (!entityToFollow) {
        return;
    }
    const b2Vec2 currentOffsetWorld = window.getOffsetWorld();
    const b2Vec2 camPos = {-currentOffsetWorld.x, currentOffsetWorld.y};
    const b2Vec2 entityPos = b2Body_GetPosition(entityToFollow->getBodyId());
    const b2Vec2 windowSizeWorld = window.getSizeWorld();
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
    window.updateOffset(newCamPos);
}

void Camera::applyViewableLimits(b2Vec2& camPos) {
    const b2Vec2 windowSizeWorld = window.getSizeWorld();
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
    window.updateOffset(entityToFollow->getPosition());
}
