#pragma once
#include "Camera.hpp"
#include "Colors.hpp"
#include "WindowManager.hpp"
#include <SDL3_ttf/SDL_ttf.h>

namespace Drawing {
void polygon(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    SDL_FColor color = colorToFColor(Colors.White)
);
void polygonBorders(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    SDL_FColor color = colorToFColor(Colors.Black)
);
void showFanTriangulation(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    SDL_FColor color = colorToFColor(Colors.Red)
);
void rectangleBorders(
    b2Vec2 min,
    b2Vec2 max,
    WindowManager& window,
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    SDL_FColor color
);
void text(
    TTF_Text* text,
    WindowManager& window,
    b2Vec2 worldPosition,
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    // Next two params are consts from AssetManager
    float textRenderScale,
    float textWorldSizeMultiplier,
    SDL_FColor textColor = colorToFColor(Colors.White),
    std::optional<SDL_FColor> backgroundColor = SDL_FColor{0.f, 0.f, 0.f, 0.5f}
);
void texture(
    SDL_Texture* texture,
    WindowManager& window,
    b2Vec2 worldPosition,
    b2Vec2 worldSize,
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    double sdlAngle = 0.0,
    SDL_FlipMode flip = SDL_FLIP_NONE
);
double b2RotToSdlAngle(b2Rot rotation);
b2Vec2 getTextWorldSize(TTF_Text* text, float textRenderScale, float textWorldSizeMultiplier);
bool shouldDrawObject(
    b2Vec2 objectPosBottomLeft, b2Vec2 objectSize, float minX, float maxX, float minY, float maxY
);
}