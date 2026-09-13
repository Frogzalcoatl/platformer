#include "gui/ui_manager.h"

void UiManager::draw_main_menu(WindowManager& window) {
    float menu_height = 250.f * ui_scale_;
    draw_large_logo(window, menu_height);
    SDL_Rect safe_area = window.get_safe_area();
    SDL_FRect safe_area_f{
        static_cast<float>(safe_area.x),
        static_cast<float>(safe_area.y),
        static_cast<float>(safe_area.w),
        static_cast<float>(safe_area.h)
    };
    WindowVec2 window_size = window.get_size();
    ImVec2 window_size_f{static_cast<float>(window_size.x), static_cast<float>(window_size.y)};
    float absolute_center_x = window_size_f.x * 0.5f;
    ImVec2 menu_size = ImVec2{300.f * ui_scale_, menu_height};
    float logo_top = safe_area_f.y + logo_top_padding_;
    float logo_bottom = logo_top + logo_height_;
    float logo_menu_spacing = 15.f * ui_scale_;
    float actual_menu_top = logo_bottom + logo_menu_spacing;
    ImGui::SetNextWindowPos(
        ImVec2{absolute_center_x, actual_menu_top}, ImGuiCond_Always, ImVec2{0.5f, 0.0f}
    );
    ImGui::SetNextWindowSize(menu_size);
    if (ImGui::Begin(
            "Main Menu",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoBringToFrontOnFocus
        )) {
        ImGui::PushFont(font_large_);
        float vertical_spacing = 15.f * ui_scale_;
        float window_width = ImGui::GetWindowSize().x;
        float button_width = 225.f * ui_scale_;
        float button_height = 50.f * ui_scale_;
        float cursor_x = (window_width - button_width) * 0.5f;
        ImGui::SetCursorPosY(25.f * ui_scale_);
        ImGui::SetCursorPosX(cursor_x);
        if (ImGui::Button("Test Game", ImVec2{button_width, button_height})) {
            set_state(UiState::player_source_setup);
        }
        ImGui::SetItemDefaultFocus();
        apply_hover_sounds();
        apply_click_sounds();
        ImGui::Dummy(ImVec2(0, vertical_spacing));
        ImGui::SetCursorPosX(cursor_x);
        if (ImGui::Button("Settings", ImVec2{button_width, button_height})) {
            set_state(UiState::settings);
        }
        apply_hover_sounds();
        apply_click_sounds();
        ImGui::Dummy(ImVec2(0, vertical_spacing));
        ImGui::SetCursorPosX(cursor_x);
        if (ImGui::Button("Quit", ImVec2{button_width, button_height})) {
            game_events::push(game_event_types::CloseWindow{});
        }
        apply_hover_sounds();
        apply_click_sounds();
        ImGui::PopFont();
    }
    ImGui::End();
}