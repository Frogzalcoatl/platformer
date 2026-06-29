#include "DrawPolygon.hpp"
#include <array>
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

void drawPolygon(
    const b2Polygon& polygon, b2Transform& transform, WindowManager& window, SDL_FColor color
) {
    std::array<SDL_Vertex, B2_MAX_POLYGON_VERTICES> sdlVertices = {0};
    for (int i = 0; i < polygon.count; i++) {
        sdlVertices[i].color = color;
        sdlVertices[i].position = scaleB2Point(window, transform, polygon.vertices[i]);
    }
    std::vector<int> indices = fanTriangulation(polygon.count);
    SDL_RenderGeometry(
        window.sdlRenderer, NULL, &sdlVertices.front(), polygon.count, &indices.front(),
        static_cast<int>(indices.size())
    );
}