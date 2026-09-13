#include "gui/notification_manager.h"
#include <imgui.h>

void NotificationManager::remove_index(size_t i) {
    if (i >= notifications_.size()) {
        return;
    }
    // std::ptrdiff_t "Pointer Difference" i think
    // Just added to avoid warnings about num type
    notifications_.erase(notifications_.begin() + static_cast<std::ptrdiff_t>(i));
}

void NotificationManager::send(std::string_view message, std::function<void()> on_click) {
    Notification noti;
    noti.message = std::string{message};
    noti.timestamp = SDL_GetTicks();
    noti.on_click = std::move(on_click);
    notifications_.push_back(noti);
}

void NotificationManager::update(WindowManager& window, const float ui_scale) {
    const Uint64 duration_ms = duration_seconds * 1000;
    const Uint64 now = SDL_GetTicks();
    while (!notifications_.empty() && notifications_.begin()->timestamp + duration_ms <= now) {
        // Elements should always be in order of timestamp so this should always work
        notifications_.erase(notifications_.begin());
    }
    std::erase_if(notifications_, [](const Notification& n) { return n.dismissed; });
    draw(window, ui_scale);
}

void NotificationManager::draw(WindowManager& window, const float ui_scale) {
    if (notifications_.empty()) {
        return;
    }
    const SDL_Rect safe_area = window.get_safe_area();
    const ImVec2 window_pos{
        static_cast<float>(safe_area.x + safe_area.w), static_cast<float>(safe_area.y)
    };
    const float button_width = 250.f * ui_scale;
    const float x_button_width = 50.f * ui_scale;
    ImGuiStyle& style = ImGui::GetStyle();
    const float wrap_width = button_width - (style.FramePadding.x * 2.f);
    const float min_height = 50.f * ui_scale;
    const float vertical_spacing = 10.f * ui_scale;
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                                   ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2{1.f, 0.f});
    if (ImGui::Begin("Notifications", nullptr, flags)) {
        for (size_t i = 0; i < notifications_.size(); i++) {
            const ImVec2 text_size =
                ImGui::CalcTextSize(notifications_[i].message.c_str(), nullptr, false, wrap_width);
            const float calculated_height = text_size.y + (style.FramePadding.y * 2.f);
            const float final_height =
                (calculated_height > min_height) ? calculated_height : min_height;
            const ImVec2 notification_size{button_width, final_height};
            ImVec2 original_cursor_pos = ImGui::GetCursorPos();
            const std::string button_id = "##Notifications_" + std::to_string(i);
            if (ImGui::Button(button_id.c_str(), notification_size)) {
                if (notifications_[i].on_click) {
                    notifications_[i].dismissed = true;
                    notifications_[i].on_click();
                }
            }
            float text_offset_y = (final_height - text_size.y) * 0.5f;
            ImGui::SetCursorPos(
                ImVec2{
                    original_cursor_pos.x + style.FramePadding.x,
                    original_cursor_pos.y + text_offset_y
                }
            );
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + wrap_width);
            ImGui::TextUnformatted(notifications_[i].message.c_str());
            ImGui::PopTextWrapPos();
            ImGui::SetCursorPos(
                ImVec2{original_cursor_pos.x + button_width, original_cursor_pos.y}
            );
            std::string x_label = "X##NotificationsX_" + std::to_string(i);
            if (ImGui::Button(x_label.c_str(), ImVec2{x_button_width, final_height})) {
                notifications_[i].dismissed = true;
            }
            ImGui::SetCursorPos(
                ImVec2{
                    original_cursor_pos.x, original_cursor_pos.y + final_height + vertical_spacing
                }
            );
            ImGui::Dummy(ImVec2{0.f, 0.f});
        }
    }
    ImGui::End();
}