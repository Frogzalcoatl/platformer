#include "gui/UiManager.hpp"

void UiManager::drawDebug(
    WindowManager& window, Entity* playerEntity, Camera* camera, InputManager& input, Level* level
) {
    ImGui::PushFont(fontSmall);
    SDL_Rect safeArea = window.getSafeArea();
    ImGui::SetNextWindowPos(
        ImVec2{static_cast<float>(safeArea.x), static_cast<float>(safeArea.y)}, ImGuiCond_Always
    );
    if (ImGui::Begin(
            "Debug Menu",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize
        )) {
        ImGui::Text("Window:");
        WindowVec2 windowSize = window.getSize();
        ImGui::Text("Size: %d, %d", windowSize.x, windowSize.y);
        ImGui::Text("Framerate:");
        ImGui::SameLine();
        fpsText(window);
        ImGui::Dummy(ImVec2{1.f, 1.f});
        ImGui::Text("\nUI State: %s", getStateStr().c_str());
        ImGui::Dummy(ImVec2{1.f, 1.f});
        if (level) {
            std::string_view levelName = level->getName();
            LevelDimensions levelSize = level->getSize();
            LevelDrawInfo drawInfo = level->drawnLastFrame();
            size_t tileCount = level->getTileCount();
            size_t entitiesCount = level->getEntities().size();
            ImGui::Text("\nLevel:");
            ImGui::Text(
                // %.*s tells the func to read exactly N characters, preventing it from running past
                // the end of a string_view
                "Name: \"%.*s\"\nSize: (%zu, %zu)\nTiles Drawn: %zu/%zu\nEntities Drawn: %zu/%zu",
                static_cast<int>(levelName.length()),
                levelName.data(),
                levelSize.width,
                levelSize.height,
                drawInfo.tiles,
                tileCount,
                drawInfo.entities,
                entitiesCount
            );
            ImGui::Dummy(ImVec2{1.f, 1.f});
        }
        if (playerEntity) {
            b2Vec2 position = b2Body_GetPosition(playerEntity->getBodyId());
            b2Vec2 velocity = b2Body_GetLinearVelocity(playerEntity->getBodyId());
            ImGui::Text("\nPlayer 1:");
            ImGui::Text(
                "Position: %.2f, %.2f\nVelocity: %.2f, %.2f",
                position.x,
                position.y,
                velocity.x,
                velocity.y
            );
            ImGui::Dummy(ImVec2{1.f, 1.f});
        }
        if (camera) {
            const b2Vec2 offsetWorld = camera->getOffsetWorld();
            const WindowVec2 offsetPixels = camera->getOffsetPixels();
            const b2Vec2 size = camera->getSize();
            const b2Vec2 safeAreaSize = camera->getSafeAreaSize();
            const b2Vec2 safeAreaValue = camera->getEntitySafeAreaValue();
            const b2Vec2 mouseWorldPos = camera->pixelPosToWorldPos(window.getMousePos());
            const float scaleMultiplier = camera->getScaleMultiplier();
            ImGui::Text("\nCamera:");
            ImGui::Text(
                "Zoom: %.2f\nOffset Pixels: %d, %d\nOffset World: %.2f, %.2f\nSize World: %.2f, %.2f\nSafe Area Size World: %.2f, %.2f\nPlayer Ratio from Center: %.2f, %.2f\nMouse Position World: %.2f, %.2f",
                scaleMultiplier,
                offsetPixels.x,
                offsetPixels.y,
                offsetWorld.x,
                offsetWorld.y,
                size.x,
                size.y,
                safeAreaSize.x,
                safeAreaSize.y,
                safeAreaValue.x,
                safeAreaValue.y,
                mouseWorldPos.x,
                mouseWorldPos.y
            );
            ImGui::Dummy(ImVec2{1.f, 1.f});
        }
        int sdlGamepadCount = input.sdlGamepadsDetected();
        ImGui::Text("\nSDL Gamepads Detected: %d", sdlGamepadCount);
        size_t playerSourceCount = input.getPlayerSourceCount();
        size_t maxPlayerSourceCount = input.getPlayerSources().size();
        ImGui::Text("Player Sources Connected: %zu/%zu", playerSourceCount, maxPlayerSourceCount);
        ImGui::PopFont();
    }
    ImGui::End();
}