#include "Drawing.hpp"
#include <array>
#include <cassert>
#include <vector>

static SDL_FPoint scaleB2Point(WindowManager& window, b2Transform transform, b2Vec2 point) {
    b2Vec2 worldPosition = b2TransformPoint(transform, point);
    float scaleFactor = window.getScaleFactor();
    WindowDimensions offset = window.getOffsetPixels();
    worldPosition.x *= scaleFactor;
    worldPosition.y *= scaleFactor;
    worldPosition.y *= -1.f;
    worldPosition.x += offset.x;
    worldPosition.y += offset.y;
    return SDL_FPoint{worldPosition.x, worldPosition.y};
}

// Returns indices for triangles
static std::vector<int> fanTriangulation(int vertexCount) {
    std::vector<int> indices;
    for (int current = 2; current <= vertexCount - 1; current++) {
        indices.push_back(0);
        indices.push_back(current - 1);
        indices.push_back(current);
    }
    return indices;
}

void Drawing::polygon(
    const b2Polygon& polygon, b2Transform& transform, WindowManager& window, SDL_FColor color
) {
    assert(polygon.count >= 3);
    std::array<SDL_Vertex, B2_MAX_POLYGON_VERTICES> sdlVertices = {0};
    for (int i = 0; i < polygon.count; i++) {
        sdlVertices[i].color = color;
        sdlVertices[i].position = scaleB2Point(window, transform, polygon.vertices[i]);
    }
    std::vector<int> indices = fanTriangulation(polygon.count);
    SDL_RenderGeometry(
        window.sdlRenderer,
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

// Load big font scale down for passable text resolution even when zoomed in
static const float WorldTextScaling = 0.005f;

void Drawing::text(
    TTF_Text* text,
    b2Vec2 worldPosition,
    WindowManager& window,
    SDL_FColor textColor,
    std::optional<SDL_FColor> backgroundColor
) {
    if (text == nullptr) {
        return;
    }
    float oldRenderScaleX, oldRenderScaleY;
    SDL_GetRenderScale(window.sdlRenderer, &oldRenderScaleX, &oldRenderScaleY);
    float scaleFactor = window.getScaleFactor();
    scaleFactor *= WorldTextScaling;
    SDL_SetRenderScale(window.sdlRenderer, scaleFactor, scaleFactor);
    WindowDimensions offsetPixels = window.getOffsetPixels();
    b2Vec2 windowPosition;
    windowPosition.x = worldPosition.x / WorldTextScaling + offsetPixels.x / scaleFactor;
    windowPosition.y = worldPosition.y * -1.f / WorldTextScaling + offsetPixels.y / scaleFactor;
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
            SDL_GetRenderDrawBlendMode(window.sdlRenderer, &oldBlendMode);
            SDL_SetRenderDrawBlendMode(window.sdlRenderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColorFloat(
                window.sdlRenderer,
                backgroundColor.value().r,
                backgroundColor.value().g,
                backgroundColor.value().b,
                backgroundColor.value().a
            );
            SDL_RenderFillRect(window.sdlRenderer, &backgroundRect);
            SDL_SetRenderDrawBlendMode(window.sdlRenderer, oldBlendMode);
        }
    }
    TTF_SetTextColorFloat(text, textColor.r, textColor.g, textColor.b, textColor.a);
    TTF_DrawRendererText(text, windowPosition.x, windowPosition.y);
    SDL_SetRenderScale(window.sdlRenderer, oldRenderScaleX, oldRenderScaleY);
}