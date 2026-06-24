#pragma once
#include <SDL3/SDL.h>

inline SDL_FColor colorToFColor(int r, int g, int b) { return SDL_FColor{r / 255.f, g / 255.f, b / 255.f, 1.f}; }

inline struct Colors {
    SDL_FColor GrassGreen = colorToFColor(0, 154, 23);
    SDL_FColor Gray = colorToFColor(200, 200, 200);
    SDL_FColor Brown = colorToFColor(100, 65, 23);
    SDL_FColor Purple = colorToFColor(186, 85, 211);
    SDL_Color BackGround = {20, 20, 30, 255};
} Colors;