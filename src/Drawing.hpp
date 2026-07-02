#pragma once
#include "Colors.hpp"
#include "WindowManager.hpp"
#include <SDL3_ttf/SDL_ttf.h>

namespace Drawing {
void polygon(
    const b2Polygon& polygon,
    b2Transform& transform,
    WindowManager& window,
    SDL_FColor color = colorToFColor(Colors.White)
);
void polygonBorders(
    const b2Polygon& polygon,
    b2Transform& transform,
    WindowManager& window,
    SDL_FColor color = colorToFColor(Colors.Black)
);
void showFanTriangulation(
    const b2Polygon& polygon,
    b2Transform& transform,
    WindowManager& window,
    SDL_FColor color = colorToFColor(Colors.Red)
);
void text(
    WindowManager& window,
    TTF_Text* text,
    float textResolutionScaleFactor,
    b2Vec2 worldPosition,
    SDL_FColor textColor = colorToFColor(Colors.White),
    std::optional<SDL_FColor> backgroundColor = SDL_FColor{0.f, 0.f, 0.f, 0.5f}
);
void texture(
    WindowManager& window,
    SDL_Texture* texture,
    b2Vec2 worldPosition,
    b2Vec2 worldSize,
    double sdlAngle = 0.0,
    SDL_FlipMode flip = SDL_FLIP_NONE
);
double b2RotToSdlAngle(b2Rot rotation);
} // namespace Drawing