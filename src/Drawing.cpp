#include "Drawing.hpp"
#include <SDL3_image/SDL_image.h>
#include <cassert>
#include <vector>

static std::vector<SDL_FPoint> getPolygonPoints(
    const b2Polygon& polygon, b2Transform& transform, float scaleFactor, WindowVec2 offsetPixels
) {
    std::vector<SDL_FPoint> points;
    points.reserve(polygon.count);
    for (int i = 0; i < polygon.count; i++) {
        b2Vec2 pos = b2TransformPoint(transform, polygon.vertices[i]);
        pos.x *= scaleFactor;
        pos.y *= scaleFactor;
        pos.y *= -1.f;
        pos.x += offsetPixels.x;
        pos.y += offsetPixels.y;
        points.push_back(SDL_FPoint{pos.x, pos.y});
    }
    return points;
}

// TODO: Account for polygon radius
void Drawing::polygon(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float scaleFactor,
    WindowVec2 offsetPixels,
    SDL_FColor color
) {
    assert(polygon.count >= 3);
    SDL_Renderer* renderer = window.getSdlRenderer();
    if (!renderer) {
        return;
    }
    std::vector<SDL_FPoint> points =
        getPolygonPoints(polygon, transform, scaleFactor, offsetPixels);
    std::vector<SDL_Vertex> vertices;
    for (size_t i = 0; i < points.size(); i++) {
        SDL_Vertex vertex;
        vertex.color = color;
        vertex.position = points[i];
        vertices.push_back(vertex);
    }
    // Fan triangulation
    std::vector<int> indices;
    indices.reserve(polygon.count * 3 - 2);
    for (int current = 2; current <= polygon.count - 1; current++) {
        indices.push_back(0);
        indices.push_back(current - 1);
        indices.push_back(current);
    }
    SDL_RenderGeometry(
        renderer,
        NULL,
        vertices.data(),
        polygon.count,
        indices.data(),
        static_cast<int>(indices.size())
    );
}

void Drawing::polygonBorders(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float scaleFactor,
    WindowVec2 offsetPixels,
    SDL_FColor color
) {
    assert(polygon.count >= 3);
    SDL_Renderer* renderer = window.getSdlRenderer();
    if (!renderer) {
        return;
    }
    std::vector<SDL_FPoint> points =
        getPolygonPoints(polygon, transform, scaleFactor, offsetPixels);
    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    points.push_back(points[0]);
    SDL_RenderLines(renderer, points.data(), static_cast<int>(points.size()));
}

void Drawing::showFanTriangulation(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float scaleFactor,
    WindowVec2 offsetPixels,
    SDL_FColor color
) {
    assert(polygon.count >= 3);
    SDL_Renderer* renderer = window.getSdlRenderer();
    if (!renderer) {
        return;
    }
    std::vector<SDL_FPoint> points =
        getPolygonPoints(polygon, transform, scaleFactor, offsetPixels);
    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    std::vector<SDL_FPoint> perimeter;
    perimeter.reserve(polygon.count + 1);
    perimeter.insert(perimeter.end(), points.begin(), points.end());
    perimeter.push_back(points[0]);
    SDL_RenderLines(renderer, perimeter.data(), static_cast<int>(perimeter.size()));
    for (int i = 2; i < polygon.count; i++) {
        SDL_RenderLine(renderer, points[0].x, points[0].y, points[i].x, points[i].y);
    }
}

static const float TextShrinkageMultiplier = 25.f;

void Drawing::text(
    TTF_Text* text,
    WindowManager& window,
    b2Vec2 worldPosition,
    float scaleFactor,
    WindowVec2 offsetPixels,
    float textResolutionScaleFactor,
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
    textResolutionScaleFactor *= TextShrinkageMultiplier;
    scaleFactor /= textResolutionScaleFactor;
    SDL_SetRenderScale(renderer, scaleFactor, scaleFactor);
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
    SDL_Texture* texture,
    WindowManager& window,
    b2Vec2 worldPosition,
    b2Vec2 worldSize,
    float scaleFactor,
    WindowVec2 offsetPixels,
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
    SDL_FRect rect;
    rect.w = worldSize.x * scaleFactor;
    rect.h = worldSize.y * scaleFactor;
    rect.x = (worldPosition.x - worldSize.x / 2.f) * scaleFactor + offsetPixels.x;
    rect.y = (worldPosition.y + worldSize.y / 2.f) * scaleFactor * -1.f + offsetPixels.y;
    SDL_RenderTextureRotated(renderer, texture, nullptr, &rect, sdlAngle, nullptr, flip);
}

double Drawing::b2RotToSdlAngle(b2Rot rotation) {
    float radians = b2Rot_GetAngle(rotation);
    return -static_cast<double>(radians) * (180.0 / SDL_PI_D);
}