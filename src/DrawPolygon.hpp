#pragma once
#include "WindowManager.hpp"

void drawPolygon(
    const b2Polygon& polygon, b2Transform& transform, WindowManager& window,
    SDL_FColor color = SDL_FColor{1.f, 0.f, 0.f, 1.f}
);