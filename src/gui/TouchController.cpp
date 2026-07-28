#include "gui/TouchController.hpp"
#include "assets/AssetPaths.hpp"
#include "gui/ImGuiWidgets.hpp"
#include "platformer/GameEvents.hpp"
#include <vector>

TouchController::TouchController() {
    entityController.isSprinting = true;
}

TouchController::TouchController(Entity& entity) : entityController(entity) {
    entityController.isSprinting = true;
}

bool TouchController::isLastItemTouched(const std::vector<ImVec2>& touchPositions) {
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    for (const ImVec2& pos : touchPositions) {
        if (pos.x >= min.x && pos.x <= max.x && pos.y >= min.y && pos.y <= max.y) {
            return true;
        }
    }
    if (touchPositions.empty()) {
        // For testing with mouse input
        return ImGui::IsItemActive();
    }
    return false;
}

void TouchController::draw(WindowManager& window, float uiScale) {
    ImGui::Begin(
        "Touch Controls",
        nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize
    );
    WindowVec2 windowSize = window.getSize();
    ImVec2 windowSizeF{static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)};
    std::vector<ImVec2> activeTouchPositions;
    int touchDeviceCount = 0;
    SDL_TouchID* touchDevices = SDL_GetTouchDevices(&touchDeviceCount);
    if (touchDevices) {
        for (int i = 0; i < touchDeviceCount; i++) {
            SDL_TouchDeviceType type = SDL_GetTouchDeviceType(touchDevices[i]);
            if (type != SDL_TOUCH_DEVICE_DIRECT) {
                continue;
            }
            int fingerCount;
            SDL_Finger** fingers = SDL_GetTouchFingers(touchDevices[i], &fingerCount);
            if (fingers) {
                for (int j = 0; j < fingerCount; j++) {
                    // Multiply by window size since the coordinates are from 0.0-1.0
                    activeTouchPositions.push_back(
                        ImVec2{fingers[j]->x * windowSizeF.x, fingers[j]->y * windowSizeF.y}
                    );
                }
                SDL_free(fingers);
            }
        }
        SDL_free(touchDevices);
    }
    SDL_Rect safeArea = window.getSafeArea();
    SDL_FRect safeAreaF{
        static_cast<float>(safeArea.x),
        static_cast<float>(safeArea.y),
        static_cast<float>(safeArea.w),
        static_cast<float>(safeArea.h)
    };
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 0.40f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 0.60f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.40f, 0.40f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 1.00f, 1.00f, 0.45f));
    std::vector<TouchButtonRect> buttonRects;
    ImVec2 pauseButtonSize{100.f * uiScale, 100.f * uiScale};
    ImVec2 pauseButtonPos{windowSizeF.x / 2.f - pauseButtonSize.x, 0.f};
    ImGui::SetCursorPos(pauseButtonPos);
    ImGuiWidgets::CustomPauseButton("##Pause", pauseButtonSize);
    buttonRects.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax()});
    bool isPauseTouched = isLastItemTouched(activeTouchPositions);
    if (isPauseTouched && !wasPauseTouched) {
        GameEvents::Push(
            GameEventTypes::Input{
                InputVerb::Pause, InputState::Pressed, InputSource{InputType::Touch, 0}
            }
        );
    }
    wasPauseTouched = isPauseTouched;
    ImVec2 buttonSize = ImVec2{100.f * uiScale, 100.f * uiScale};
    ImVec2 upButtonPos{safeAreaF.w - buttonSize.x, safeAreaF.y + safeAreaF.h - buttonSize.y * 2.f};
    ImGui::SetCursorPos(upButtonPos);
    ImGuiWidgets::CustomArrowButton("##Up", ImGuiDir_Up, buttonSize);
    buttonRects.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax()});
    bool isUpTouched = isLastItemTouched(activeTouchPositions);
    if (isUpTouched && !wasUpTouched) {
        entityController.movement[static_cast<size_t>(EntityMovement::Up)] = true;
        entityController.jump();
        float pitch = SDL_randf() * (1.25f - 1.f) + 1.f;
        GameEvents::Push(GameEventTypes::PlaySound{AssetPaths::Sounds::Jump, 100, pitch});
    } else if (!isUpTouched && wasUpTouched) {
        entityController.movement[static_cast<size_t>(EntityMovement::Up)] = false;
    }
    wasUpTouched = isUpTouched;
    ImVec2 downButtonPos{upButtonPos.x, upButtonPos.y + buttonSize.y};
    ImGui::SetCursorPos(downButtonPos);
    ImGuiWidgets::CustomArrowButton("##Down", ImGuiDir_Down, buttonSize);
    buttonRects.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax()});
    entityController.movement[static_cast<size_t>(EntityMovement::Down)] =
        isLastItemTouched(activeTouchPositions);
    ImVec2 leftButtonPos{static_cast<float>(safeArea.x), downButtonPos.y};
    ImGui::SetCursorPos(leftButtonPos);
    ImGuiWidgets::CustomArrowButton("##Left", ImGuiDir_Left, buttonSize);
    buttonRects.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax()});
    entityController.movement[static_cast<size_t>(EntityMovement::Left)] =
        isLastItemTouched(activeTouchPositions);
    ImVec2 rightButtonPos{leftButtonPos.x + buttonSize.x, leftButtonPos.y};
    ImGui::SetCursorPos(rightButtonPos);
    ImGuiWidgets::CustomArrowButton("##Right", ImGuiDir_Right, buttonSize);
    buttonRects.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax()});
    entityController.movement[static_cast<size_t>(EntityMovement::Right)] =
        isLastItemTouched(activeTouchPositions);
    ImGui::PopStyleColor(4);
    ImGui::End();
    int uiFingers = 0;
    for (const ImVec2& pos : activeTouchPositions) {
        bool onUi = false;
        for (const auto& rect : buttonRects) {
            if (pos.x >= rect.min.x && pos.x <= rect.max.x && pos.y >= rect.min.y &&
                pos.y <= rect.max.y) {
                onUi = true;
                break;
            }
        }
        if (onUi) {
            uiFingers++;
        }
    }
    freeFingerCount = static_cast<int>(activeTouchPositions.size()) - uiFingers;
    entityController.update();
}