#include "debugUi.hpp"
#include "windowManager.hpp"
#include <box2d/box2d.h>

void drawUI(WindowManager* window, Player* player) {
    ImGui::Begin("Debug Menu");
    ImGui::Text("\nApplication average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    WindowDimensions offset = window->getOffset();
    WindowDimensions windowSize = window->getSize();
    int scaleFactor = window->getScaleFactor();
    ImGui::Text("\nWindow:\nSize: %d, %d\nRender Offset: %d, %d\nScale Factor: %d", windowSize.x, windowSize.y,
                offset.x, offset.y, scaleFactor);
    b2Vec2 position = b2Body_GetPosition(player->bodyId);
    b2Vec2 velocity = b2Body_GetLinearVelocity(player->bodyId);
    ImGui::Text("\nPlayer:\nInput: %d %d %d %d\nPosition: %.2f, %.2f\nVelocity: %.2f, %.2f", player->movement.up,
                player->movement.down, player->movement.left, player->movement.right, position.x, position.y,
                velocity.x, velocity.y);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), window->sdlRenderer);
}