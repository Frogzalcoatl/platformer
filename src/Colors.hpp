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

constexpr SDL_Color hexToColor(Uint32 hex) {
    if (hex <= 0xFFFFFF) {
        return SDL_Color{
            static_cast<Uint8>((hex >> 16) & 0xFF),
            static_cast<Uint8>((hex >> 8) & 0xFF),
            static_cast<Uint8>(hex & 0xFF),
            255
        };
    }
    return SDL_Color{
        static_cast<Uint8>((hex >> 24) & 0xFF),
        static_cast<Uint8>((hex >> 16) & 0xFF),
        static_cast<Uint8>((hex >> 8) & 0xFF),
        static_cast<Uint8>(hex & 0xFF)
    };
}

namespace Colors {
inline constexpr SDL_Color GrassGreen = {0, 154, 23, 255};
inline constexpr SDL_Color Gray = {200, 200, 200, 255};
inline constexpr SDL_Color Brown = {100, 65, 23, 255};
inline constexpr SDL_Color Purple = {186, 85, 211, 255};
inline constexpr SDL_Color Background = {20, 20, 30, 255};
inline constexpr SDL_Color White = {255, 255, 255, 255};
inline constexpr SDL_Color Red = {255, 0, 0, 255};
inline constexpr SDL_Color Black = {0, 0, 0, 255};
inline constexpr SDL_Color Yellow = {255, 255, 0, 255};
inline constexpr SDL_Color SkyBlue = hexToColor(0x0082C8);
inline constexpr SDL_Color Blue = {0, 0, 255, 255};
}
