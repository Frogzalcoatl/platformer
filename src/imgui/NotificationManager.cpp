#include "imgui/NotificationManager.hpp"
#include <imgui.h>

void NotificationManager::removeIndex(size_t i) {
    if (i >= notifications.size()) {
        return;
    }
    // std::ptrdiff_t "Pointer Difference" i think
    // Just added to avoid warnings about num type
    notifications.erase(notifications.begin() + static_cast<std::ptrdiff_t>(i));
}

void NotificationManager::send(std::string_view message, std::function<void()> onClick) {
    Notification noti;
    noti.message = std::string{message};
    noti.timestamp = SDL_GetTicks();
    noti.onClick = std::move(onClick);
    notifications.push_back(noti);
}

void NotificationManager::update(WindowManager& window, const float uiScale) {
    const Uint64 durationMS = durationSeconds * 1000;
    const Uint64 now = SDL_GetTicks();
    while (!notifications.empty() && notifications.begin()->timestamp + durationMS <= now) {
        // Elements should always be in order of timestamp so this should always work
        notifications.erase(notifications.begin());
    }
    std::erase_if(notifications, [](const Notification& n) { return n.dismissed; });
    draw(window, uiScale);
}

void NotificationManager::draw(WindowManager& window, const float uiScale) {
    if (notifications.empty()) {
        return;
    }
    const SDL_Rect safeArea = window.getSafeArea();
    const ImVec2 windowPos{
        static_cast<float>(safeArea.x + safeArea.w), static_cast<float>(safeArea.y)
    };
    const float buttonWidth = 250.f * uiScale;
    const float xButtonWidth = 50.f * uiScale;
    ImGuiStyle& style = ImGui::GetStyle();
    const float wrapWidth = buttonWidth - (style.FramePadding.x * 2.f);
    const float minHeight = 50.f * uiScale;
    const float verticalSpacing = 10.f * uiScale;
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                                   ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2{1.f, 0.f});
    if (ImGui::Begin("Notifications", nullptr, flags)) {
        for (size_t i = 0; i < notifications.size(); i++) {
            const ImVec2 textSize =
                ImGui::CalcTextSize(notifications[i].message.c_str(), nullptr, false, wrapWidth);
            const float calculatedHeight = textSize.y + (style.FramePadding.y * 2.f);
            const float finalHeight = (calculatedHeight > minHeight) ? calculatedHeight : minHeight;
            const ImVec2 notificationSize{buttonWidth, finalHeight};
            ImVec2 originalCursorPos = ImGui::GetCursorPos();
            const std::string buttonId = "##Notifications_" + std::to_string(i);
            if (ImGui::Button(buttonId.c_str(), notificationSize)) {
                if (notifications[i].onClick) {
                    notifications[i].dismissed = true;
                    notifications[i].onClick();
                }
            }
            float textOffsetY = (finalHeight - textSize.y) * 0.5f;
            ImGui::SetCursorPos(
                ImVec2{
                    originalCursorPos.x + style.FramePadding.x, originalCursorPos.y + textOffsetY
                }
            );
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + wrapWidth);
            ImGui::TextUnformatted(notifications[i].message.c_str());
            ImGui::PopTextWrapPos();
            ImGui::SetCursorPos(ImVec2{originalCursorPos.x + buttonWidth, originalCursorPos.y});
            std::string xLabel = "X##NotificationsX_" + std::to_string(i);
            if (ImGui::Button(xLabel.c_str(), ImVec2{xButtonWidth, finalHeight})) {
                notifications[i].dismissed = true;
            }
            ImGui::SetCursorPos(
                ImVec2{originalCursorPos.x, originalCursorPos.y + finalHeight + verticalSpacing}
            );
            ImGui::Dummy(ImVec2{0.f, 0.f});
        }
    }
    ImGui::End();
}