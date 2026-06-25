#pragma once
#include <SDL3/SDL.h>

inline SDL_FColor colorToFColor(int r, int g, int b) { return SDL_FColor{r / 255.f, g / 255.f, b / 255.f, 1.f}; }

inline struct Colors {
    SDL_Color GrassGreen = {0, 154, 23, 255};
    SDL_Color Gray = {200, 200, 200, 255};
    SDL_Color Brown = {100, 65, 23, 255};
    SDL_Color Purple = {186, 85, 211, 255};
    SDL_Color BackGround = {20, 20, 30, 255};
} Colors;