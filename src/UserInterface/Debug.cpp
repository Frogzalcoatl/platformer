#include "./Debug.hpp"
#include <imgui.h>

void UserInterface::debug(
    WindowManager& window,
    Entity* player,
    EntityController& entityController,
    Camera& camera,
    InputManager& input,
    bool& showFanTriangulation
) {
    ImGui::Begin("Debug Menu");
    ImGui::Text("Renderer:");
    ImGui::Text(
        "%.1f/%s FPS (%.3f ms/frame)",
        ImGui::GetIO().Framerate,
        window.targetFpsStr().c_str(),
        1000.0f / ImGui::GetIO().Framerate
    );
    ImGui::Checkbox("Show Triangles", &showFanTriangulation);
    bool vsync = window.isVsyncEnabled();
    if (ImGui::Checkbox("Vsync", &vsync)) {
        window.setVsync(vsync);
    }
    bool fpsUnlimited = window.getFpsUnlimited();
    if (ImGui::Checkbox("FPS Unlimited", &fpsUnlimited)) {
        window.setFpsUnlimited(fpsUnlimited);
    }
    if (!vsync && !fpsUnlimited) {
        static int tempFps = window.getTargetFps();
        ImGui::SliderInt("Target FPS", &tempFps, 10, 250);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            window.setTargetFps(tempFps);
        }
    }
    ImGui::Dummy(ImVec2{1.f, 1.f});
    WindowDimensions offset = window.getOffsetPixels();
    WindowDimensions windowSizePixels = window.getSizePixels();
    b2Vec2 windowSizeWorld = window.getSizeWorld();
    float scaleFactor = window.getScaleFactor();
    ImGui::Text("\nWindow:");
    ImGui::Text(
        "Size Pixels: %d, %d\nSize World: %.1f, %.1f\nRender Offset: %d, "
        "%d\nScale: %.2f (%.2f Pixels / Meter)",
        windowSizePixels.x,
        windowSizePixels.y,
        windowSizeWorld.x,
        windowSizeWorld.y,
        offset.x,
        offset.y,
        window.scaleMultiplier,
        scaleFactor
    );
    if (player) {
        b2Vec2 position = b2Body_GetPosition(player->getBodyId());
        b2Vec2 velocity = b2Body_GetLinearVelocity(player->getBodyId());
        ImGui::Text("\nPlayer:");
        ImGui::Text(
            "Position: %.2f, %.2f\nVelocity: %.2f, %.2f\nInput: Up %d, Down %d, Left %d, Right %d, Sprint %d",
            position.x,
            position.y,
            velocity.x,
            velocity.y,
            entityController.movement[static_cast<size_t>(EntityMovement::Up)],
            entityController.movement[static_cast<size_t>(EntityMovement::Down)],
            entityController.movement[static_cast<size_t>(EntityMovement::Left)],
            entityController.movement[static_cast<size_t>(EntityMovement::Right)],
            entityController.isSprinting
        );
    }
    if (camera.entityToFollow) {
        b2Vec2 safeAreaSize = camera.getSafeAreaSize();
        b2Vec2 safeAreaValue = camera.getEntitySafeAreaValue();
        ImGui::Text("\nSafe Area:");
        ImGui::Text(
            "Size: %.2f, %.2f\nRatio from Center: %.2f, %.2f",
            safeAreaSize.x,
            safeAreaSize.y,
            safeAreaValue.x,
            safeAreaValue.y
        );
    }
    size_t controllersConnected = input.getGamepadCount();
    ImGui::Text("\nControllers Connected: %zu", controllersConnected);
    ImGui::End();
}