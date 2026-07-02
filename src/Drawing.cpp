#include "Drawing.hpp"
#include <SDL3_image/SDL_image.h>
#include <array>
#include <cassert>
#include <vector>

void Drawing::polygon(
    const b2Polygon& polygon, b2Transform& transform, WindowManager& window, SDL_FColor color
) {
    assert(polygon.count >= 3);
    SDL_Renderer* renderer = window.getSdlRenderer();
    if (!renderer) {
        return;
    }
    const float scaleFactor = window.getScaleFactor();
    const WindowDimensions offset = window.getOffsetPixels();
    std::array<SDL_Vertex, B2_MAX_POLYGON_VERTICES> sdlVertices;
    for (int i = 0; i < polygon.count; i++) {
        sdlVertices[i].color = color;
        b2Vec2 worldPosition = b2TransformPoint(transform, polygon.vertices[i]);
        worldPosition.x *= scaleFactor;
        worldPosition.y *= scaleFactor;
        worldPosition.y *= -1.f;
        worldPosition.x += offset.x;
        worldPosition.y += offset.y;
        sdlVertices[i].position = SDL_FPoint{worldPosition.x, worldPosition.y};
    }
    // Fan triangulation
    std::vector<int> indices;
    for (int current = 2; current <= polygon.count - 1; current++) {
        indices.push_back(0);
        indices.push_back(current - 1);
        indices.push_back(current);
    }
    SDL_RenderGeometry(
        renderer,
        NULL,
        sdlVertices.data(),
        polygon.count,
        indices.data(),
        static_cast<int>(indices.size())
    );
}

/* Finish later fr
// Radians are counted from the positive x-axis
static void drawArcSlice(
    SDL_Vertex center, float radius, float minRadians, float maxRadians, WindowManager& window,
    SDL_FColor color
) {}
*/

static const float TextShrinkageMultiplier = 25.f;

void Drawing::text(
    WindowManager& window,
    TTF_Text* text,
    float textResolutionScaleFactor,
    b2Vec2 worldPosition,
    SDL_FColor textColor,
    std::optional<SDL_FColor> backgroundColor
) {
    if (text == nullptr) {
        return;
    }
    SDL_Renderer* renderer = window.getSdlRenderer();
    if (!renderer) {
        return;
    }
    float oldRenderScaleX, oldRenderScaleY;
    SDL_GetRenderScale(renderer, &oldRenderScaleX, &oldRenderScaleY);
    float scaleFactor = window.getScaleFactor();
    textResolutionScaleFactor *= TextShrinkageMultiplier;
    scaleFactor /= textResolutionScaleFactor;
    SDL_SetRenderScale(renderer, scaleFactor, scaleFactor);
    WindowDimensions offsetPixels = window.getOffsetPixels();
    b2Vec2 windowPosition;
    windowPosition.x = worldPosition.x * textResolutionScaleFactor + offsetPixels.x / scaleFactor;
    windowPosition.y =
        worldPosition.y * -1.f * textResolutionScaleFactor + offsetPixels.y / scaleFactor;
    int textWidth, textHeight;
    if (TTF_GetTextSize(text, &textWidth, &textHeight)) {
        windowPosition.x -= textWidth / 2.f;
        windowPosition.y -= textHeight / 2.f;
        if (backgroundColor.has_value()) {
            SDL_FRect backgroundRect = SDL_FRect{
                windowPosition.x,
                windowPosition.y,
                static_cast<float>(textWidth),
                static_cast<float>(textHeight)
            };
            SDL_BlendMode oldBlendMode;
            SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColorFloat(
                renderer,
                backgroundColor.value().r,
                backgroundColor.value().g,
                backgroundColor.value().b,
                backgroundColor.value().a
            );
            SDL_RenderFillRect(renderer, &backgroundRect);
            SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);
        }
    }
    TTF_SetTextColorFloat(text, textColor.r, textColor.g, textColor.b, textColor.a);
    TTF_DrawRendererText(text, windowPosition.x, windowPosition.y);
    SDL_SetRenderScale(renderer, oldRenderScaleX, oldRenderScaleY);
}

void Drawing::texture(
    WindowManager& window,
    SDL_Texture* texture,
    b2Vec2 worldPosition,
    b2Vec2 worldSize,
    double sdlAngle,
    SDL_FlipMode flip
) {
    if (texture == nullptr) {
        return;
    }
    SDL_Renderer* renderer = window.getSdlRenderer();
    if (!renderer) {
        return;
    }
    float scaleFactor = window.getScaleFactor();
    WindowDimensions offset = window.getOffsetPixels();
    SDL_FRect rect;
    rect.w = worldSize.x * scaleFactor;
    rect.h = worldSize.y * scaleFactor;
    rect.x = worldPosition.x * scaleFactor + offset.x - rect.w / 2.f;
    rect.y = worldPosition.y * scaleFactor * -1.f + offset.y - rect.h / 2.f;
    SDL_RenderTextureRotated(renderer, texture, nullptr, &rect, sdlAngle, nullptr, flip);
}

double Drawing::b2RotToSdlAngle(b2Rot rotation) {
    float radians = b2Rot_GetAngle(rotation);
    return -static_cast<double>(radians) * (180.0 / 3.14159265358979323846);
}