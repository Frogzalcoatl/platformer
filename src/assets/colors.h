#pragma once
#include <SDL3/SDL.h>

inline SDL_FColor color_to_fcolor(const SDL_Color& color) {
    return SDL_FColor{color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f};
}

inline SDL_Color fcolor_to_color(const SDL_FColor& color) {
    return SDL_Color{
        static_cast<Uint8>(SDL_roundf(color.r * 255)),
        static_cast<Uint8>(SDL_roundf(color.g * 255)),
        static_cast<Uint8>(SDL_roundf(color.b * 255)),
        static_cast<Uint8>(SDL_roundf(color.a * 255))
    };
}

/*
">>"
Right shift operator, slides number over by n bits.

"&"
And operator

Example: (hex >> 16) & 0xFF
0s out all bits in the number besides last 8 bits,
which after shifting to the left 16 bits is the R number in RGB format
*/
constexpr SDL_Color hex_to_color(Uint32 hex) {
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

namespace colors {
inline constexpr SDL_Color grass_green = {0, 154, 23, 255};
inline constexpr SDL_Color gray = {200, 200, 200, 255};
inline constexpr SDL_Color brown = {100, 65, 23, 255};
inline constexpr SDL_Color purple = {186, 85, 211, 255};
inline constexpr SDL_Color background = {20, 20, 30, 255};
inline constexpr SDL_Color white = {255, 255, 255, 255};
inline constexpr SDL_Color red = {255, 0, 0, 255};
inline constexpr SDL_Color black = {0, 0, 0, 255};
inline constexpr SDL_Color yellow = {255, 255, 0, 255};
inline constexpr SDL_Color sky_blue = hex_to_color(0x0082C8);
inline constexpr SDL_Color blue = {0, 0, 255, 255};
}