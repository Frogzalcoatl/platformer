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
void text(
    TTF_Text* text,
    b2Vec2 worldPosition,
    WindowManager& window,
    SDL_FColor textColor = colorToFColor(Colors.White),
    std::optional<SDL_FColor> backgroundColor = SDL_FColor{0.f, 0.f, 0.f, 0.5f}
);
} // namespace Drawing