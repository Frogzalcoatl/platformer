#pragma once
#include <SDL3/SDL.h>

inline SDL_FColor colorToFColor(const SDL_Color& color) {
    return SDL_FColor{color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f};
}

inline SDL_Color fColorToColor(const SDL_FColor& color) {
    return SDL_Color{
        static_cast<Uint8>(SDL_roundf(color.r * 255)),
        static_cast<Uint8>(SDL_roundf(color.g * 255)),
        static_cast<Uint8>(SDL_roundf(color.b * 255)),
        static_cast<Uint8>(SDL_roundf(color.a * 255))
    };
}

inline SDL_Color hexToColor(Uint32 rgba) {
    return SDL_Color{
        static_cast<Uint8>((rgba >> 24) & 0xFF),
        static_cast<Uint8>((rgba >> 16) & 0xFF),
        static_cast<Uint8>((rgba >> 8 & 0xFF)),
        static_cast<Uint8>(rgba & 0xFF)
    };
}

inline struct Colors {
    SDL_Color GrassGreen = {0, 154, 23, 255};
    SDL_Color Gray = {200, 200, 200, 255};
    SDL_Color Brown = {100, 65, 23, 255};
    SDL_Color Purple = {186, 85, 211, 255};
    SDL_Color BackGround = {20, 20, 30, 255};
    SDL_Color White = {255, 255, 255, 255};
} Colors;