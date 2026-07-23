#include "Drawing.hpp"
#include <SDL3_image/SDL_image.h>
#include <array>
#include <cassert>

// TODO: Account for polygon radius
void Drawing::polygon(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    SDL_FColor color
) {
    assert(polygon.count >= 3 && polygon.count <= B2_MAX_POLYGON_VERTICES);
    SDL_Renderer* renderer = window.getSdlRenderer();
    if (!renderer) {
        return;
    }
    std::array<SDL_FPoint, B2_MAX_POLYGON_VERTICES> points;
    float windowHeight = static_cast<float>(window.getSize().y);
    float cameraOffsetX = static_cast<float>(cameraOffsetPixels.x);
    float cameraOffsetY = static_cast<float>(cameraOffsetPixels.y);
    for (size_t i = 0; i < static_cast<size_t>(polygon.count); i++) {
        b2Vec2 pos = b2TransformPoint(transform, polygon.vertices[i]);
        pos.x = pos.x * cameraScale - cameraOffsetX;
        pos.y = windowHeight - (pos.y * cameraScale - cameraOffsetY);
        points[i] = SDL_FPoint{pos.x, pos.y};
    }
    std::array<SDL_Vertex, B2_MAX_POLYGON_VERTICES> vertices;
    for (size_t i = 0; i < static_cast<size_t>(polygon.count); i++) {
        vertices[i].color = color;
        vertices[i].position = points[i];
    }
    // Fan triangulation
    std::vector<int> indices;
    indices.reserve(static_cast<size_t>(polygon.count) * 3 - 2);
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
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    SDL_FColor color
) {
    assert(polygon.count >= 3 && polygon.count <= B2_MAX_POLYGON_VERTICES);
    SDL_Renderer* renderer = window.getSdlRenderer();
    if (!renderer) {
        return;
    }
    std::array<SDL_FPoint, B2_MAX_POLYGON_VERTICES + 1> points;
    float windowHeight = static_cast<float>(window.getSize().y);
    float cameraOffsetX = static_cast<float>(cameraOffsetPixels.x);
    float cameraOffsetY = static_cast<float>(cameraOffsetPixels.y);
    for (size_t i = 0; i < static_cast<size_t>(polygon.count); i++) {
        b2Vec2 pos = b2TransformPoint(transform, polygon.vertices[i]);
        points[i].x = pos.x * cameraScale - cameraOffsetX;
        points[i].y = windowHeight - (pos.y * cameraScale - cameraOffsetY);
    }
    points[static_cast<size_t>(polygon.count)] = points[0];
    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderLines(renderer, points.data(), polygon.count + 1);
}

void Drawing::showFanTriangulation(
    const b2Polygon& polygon,
    WindowManager& window,
    b2Transform& transform,
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    SDL_FColor color
) {
    assert(polygon.count >= 3 && polygon.count <= B2_MAX_POLYGON_VERTICES);
    SDL_Renderer* renderer = window.getSdlRenderer();
    if (!renderer) {
        return;
    }
    std::array<SDL_FPoint, B2_MAX_POLYGON_VERTICES + 1> points;
    float windowHeight = static_cast<float>(window.getSize().y);
    float cameraOffsetX = static_cast<float>(cameraOffsetPixels.x);
    float cameraOffsetY = static_cast<float>(cameraOffsetPixels.y);
    for (size_t i = 0; i < static_cast<size_t>(polygon.count); i++) {
        b2Vec2 pos = b2TransformPoint(transform, polygon.vertices[i]);
        points[i].x = pos.x * cameraScale - cameraOffsetX;
        points[i].y = windowHeight - (pos.y * cameraScale - cameraOffsetY);
    }
    points[static_cast<size_t>(polygon.count)] = points[0];
    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderLines(renderer, points.data(), polygon.count + 1);
    for (size_t i = 2; i < static_cast<size_t>(polygon.count); i++) {
        SDL_RenderLine(renderer, points[0].x, points[0].y, points[i].x, points[i].y);
    }
}

void Drawing::rectangleBorders(
    b2Vec2 min,
    b2Vec2 max,
    WindowManager& window,
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    SDL_FColor color
) {
    SDL_Renderer* renderer = window.getSdlRenderer();
    if (!renderer) {
        return;
    }
    float windowHeight = static_cast<float>(window.getSize().y);
    float cameraOffsetX = static_cast<float>(cameraOffsetPixels.x);
    float cameraOffsetY = static_cast<float>(cameraOffsetPixels.y);
    SDL_FPoint points[5];
    points[0].x = min.x * cameraScale - cameraOffsetX;
    points[0].y = windowHeight - (min.y * cameraScale - cameraOffsetY);
    points[1].x = max.x * cameraScale - cameraOffsetX;
    points[1].y = windowHeight - (min.y * cameraScale - cameraOffsetY);
    points[2].x = max.x * cameraScale - cameraOffsetX;
    points[2].y = windowHeight - (max.y * cameraScale - cameraOffsetY);
    points[3].x = min.x * cameraScale - cameraOffsetX;
    points[3].y = windowHeight - (max.y * cameraScale - cameraOffsetY);
    points[4] = points[0];
    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderLines(renderer, points, 5);
}

void Drawing::text(
    TTF_Text* text,
    WindowManager& window,
    b2Vec2 worldPosition,
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    float textRenderScale,
    float textWorldSizeMultiplier,
    SDL_FColor textColor,
    std::optional<SDL_FColor> backgroundColor
) {
    if (!text) {
        return;
    }
    SDL_Renderer* renderer = window.getSdlRenderer();
    if (!renderer) {
        return;
    }
    int textWidthPixels, textHeightPixels;
    if (!TTF_GetTextSize(text, &textWidthPixels, &textHeightPixels)) {
        return;
    }
    float textScale = cameraScale / textRenderScale * textWorldSizeMultiplier;
    float textHeight = static_cast<float>(textHeightPixels);
    float textWidth = static_cast<float>(textWidthPixels);
    float windowHeight = static_cast<float>(window.getSize().y);
    float cameraOffsetX = static_cast<float>(cameraOffsetPixels.x);
    float cameraOffsetY = static_cast<float>(cameraOffsetPixels.y);
    SDL_FRect unscaledTextRect;
    unscaledTextRect.x =
        (worldPosition.x * cameraScale - cameraOffsetX) / textScale - (textWidth / 2.f);
    unscaledTextRect.y =
        (windowHeight - (worldPosition.y * cameraScale - cameraOffsetY)) / textScale -
        (textHeight / 2.f);
    unscaledTextRect.w = static_cast<float>(textWidthPixels);
    unscaledTextRect.h = static_cast<float>(textHeightPixels);
    float oldRenderScaleX, oldRenderScaleY;
    SDL_GetRenderScale(renderer, &oldRenderScaleX, &oldRenderScaleY);
    SDL_SetRenderScale(renderer, textScale, textScale);
    if (backgroundColor.has_value()) {
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
        SDL_RenderFillRect(renderer, &unscaledTextRect);
        SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);
    }
    TTF_SetTextColorFloat(text, textColor.r, textColor.g, textColor.b, textColor.a);
    TTF_DrawRendererText(text, unscaledTextRect.x, unscaledTextRect.y);
    SDL_SetRenderScale(renderer, oldRenderScaleX, oldRenderScaleY);
}

void Drawing::texture(
    SDL_Texture* texture,
    WindowManager& window,
    b2Vec2 worldPosition,
    b2Vec2 worldSize,
    float cameraScale,
    WindowVec2 cameraOffsetPixels,
    double sdlAngle,
    SDL_FlipMode flip
) {
    if (!texture) {
        return;
    }
    SDL_Renderer* renderer = window.getSdlRenderer();
    if (!renderer) {
        return;
    }
    float windowHeight = static_cast<float>(window.getSize().y);
    float cameraOffsetX = static_cast<float>(cameraOffsetPixels.x);
    float cameraOffsetY = static_cast<float>(cameraOffsetPixels.y);
    SDL_FRect rect;
    rect.w = worldSize.x * cameraScale;
    rect.h = worldSize.y * cameraScale;
    rect.x = (worldPosition.x - worldSize.x / 2.f) * cameraScale - cameraOffsetX;
    rect.y = windowHeight - ((worldPosition.y + worldSize.y / 2.f) * cameraScale - cameraOffsetY);
    SDL_RenderTextureRotated(renderer, texture, nullptr, &rect, sdlAngle, nullptr, flip);
}

double Drawing::b2RotToSdlAngle(b2Rot rotation) {
    float radians = b2Rot_GetAngle(rotation);
    return -static_cast<double>(radians) * (180.0 / SDL_PI_D);
}

b2Vec2
Drawing::getTextWorldSize(TTF_Text* text, float textRenderScale, float textWorldSizeMultiplier) {
    if (!text) {
        return b2Vec2{0.f, 0.f};
    }
    int textWidthPixels, textHeightPixels;
    if (!TTF_GetTextSize(text, &textWidthPixels, &textHeightPixels)) {
        return b2Vec2{0.f, 0.f};
    }
    float worldWidth =
        (static_cast<float>(textWidthPixels) / textRenderScale) * textWorldSizeMultiplier;
    float worldHeight =
        (static_cast<float>(textHeightPixels) / textRenderScale) * textWorldSizeMultiplier;
    return b2Vec2{worldWidth, worldHeight};
}

bool Drawing::shouldDrawObject(
    b2Vec2 objectPosBottomLeft, b2Vec2 objectSize, float minX, float maxX, float minY, float maxY
) {
    float objectMinX = objectPosBottomLeft.x;
    float objectMaxX = objectPosBottomLeft.x + objectSize.x;
    float objectMinY = objectPosBottomLeft.y;
    float objectMaxY = objectPosBottomLeft.y + objectSize.y;
    if (objectMinX > maxX || objectMinY > maxY || objectMaxX < minX || objectMaxY < minY) {
        return false;
    } else {
        return true;
    }
}