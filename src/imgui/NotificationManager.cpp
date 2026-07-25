#include "imgui/NotificationManager.hpp"
#include <imgui.h>

void NotificationManager::removeIndex(size_t i) {
    if (i >= notifications.size()) {
        return;
    }
    notifications.erase(notifications.begin() + i);
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
    for (size_t i = 0; i < notifications.size(); i++) {
        if (notifications[i].dismissed) {
            notifications.erase(notifications.begin() + i);
        }
    }
    draw(window, uiScale);
}

void NotificationManager::draw(WindowManager& window, const float uiScale) {
    if (notifications.empty()) {
        return;
    }
    const SDL_Rect safeArea = window.getSafeArea();
    const ImVec2 notificationSize{250.f * uiScale, 50.f * uiScale};
    const ImVec2 xButtonSize{50.f * uiScale, 50.f * uiScale};
    const ImVec2 windowPos{
        static_cast<float>(safeArea.x + safeArea.w), static_cast<float>(safeArea.y)
    };
    const float verticalSpacing = 10.f * uiScale;
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                                   ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2{1.f, 0.f});
    if (ImGui::Begin("Notifications", nullptr, flags)) {
        for (size_t i = 0; i < notifications.size(); i++) {
            std::string label = notifications[i].message + "##Notifications_" + std::to_string(i);
            if (ImGui::Button(label.c_str(), notificationSize)) {
                if (notifications[i].onClick) {
                    notifications[i].dismissed = true;
                    notifications[i].onClick();
                }
            }
            ImGui::SameLine(0.f, 0.f);
            std::string xLabel = "X##Notifications_" + std::to_string(i);
            if (ImGui::Button(xLabel.c_str(), xButtonSize)) {
                notifications[i].dismissed = true;
            }
            ImGui::Dummy(ImVec2{0.f, verticalSpacing});
        }
    }
    ImGui::End();
}