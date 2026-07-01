#pragma once
#include "Colors.hpp"
#include "WindowManager.hpp"
#include <SDL3_ttf/SDL_ttf.h>

inline double b2RotToSdlAngle(b2Rot rotation) {
    float radians = b2Rot_GetAngle(rotation);
    return -static_cast<double>(radians) * (180.0 / 3.14159265358979323846);
}

namespace Drawing {
void polygon(
    const b2Polygon& polygon,
    b2Transform& transform,
    WindowManager& window,
    SDL_FColor color = colorToFColor(Colors.White)
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
    double angle = 0.0,
    SDL_FlipMode flip = SDL_FLIP_NONE
);
} // namespace Drawing