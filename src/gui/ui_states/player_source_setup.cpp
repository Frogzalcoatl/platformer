#include "gui/ui_manager.h"

void UiManager::draw_player_source_setup(WindowManager& window, InputManager& input) {
    set_next_window_safe_area(window);
    ImGui::Begin(
        "Player Source Setup",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus
    );
    ImGui::PushFont(font_double_large_);
    ImGui::Text("Player Source Setup:");
    ImGui::PopFont();
    ImGui::PushFont(font_large_);
    ImGui::Dummy(ImVec2{0.f, 10.f * ui_scale_});
    ImGui::Text("Press any button to join!");
    static bool touch_player_enabled = input.is_touch_player_enabled(nullptr);
    if (input.has_touch_screen()) {
        ImGui::Dummy(ImVec2{0.f, 25.f * ui_scale_});
        if (ImGui::Checkbox("Touch Player", &touch_player_enabled)) {
            if (touch_player_enabled) {
                input.enable_touch_player();
            } else {
                input.disable_touch_player();
            }
            touch_player_enabled = input.is_touch_player_enabled(nullptr);
        }
        apply_hover_sounds();
        apply_click_sounds();
    }
    if (!ImGui::IsItemActive()) {
        touch_player_enabled = input.is_touch_player_enabled(nullptr);
    }
    ImGui::Dummy(ImVec2{0.f, 25.f * ui_scale_});
    const PlayerSources& player_sources = input.get_player_sources();
    for (size_t i = 0; i < player_sources.size(); i++) {
        std::string child_id = "Player " + std::to_string(i + 1);
        std::string source_name;
        if (player_sources[i].has_value()) {
            source_name = input.get_source_name(player_sources[i].value());
        } else {
            source_name = "Empty";
        }
        ImGui::Text("%s: %s", child_id.c_str(), source_name.c_str());
        if (player_sources[i].has_value()) {
            ImGui::SameLine();
            std::string button_id = "Remove##" + std::to_string(i + 1);
            if (ImGui::Button(button_id.c_str()) && !player_source_added_this_frame_) {
                input.remove_player_source_at_index(i);
            }
            apply_hover_sounds();
            apply_click_sounds();
        }
        ImGui::Dummy(ImVec2{0.f, 50.f * ui_scale_});
    }
    if (ImGui::Button("Play", ImVec2{200.f * ui_scale_, 45.f * ui_scale_})) {
        const size_t player_source_count = input.get_player_source_count();
        if (player_source_count > 0 && !player_source_added_this_frame_) {
            game_events::push(game_event_types::ShouldDetectNewPlayerSources{false});
            game_events::push(game_event_types::SetLevelName{LevelName::test});
            // To make sure the ui screen is switched after the level is loaded.
            game_events::push(game_event_types::SetUiState{UiState::playing});
        } else {
            game_events::push(
                game_event_types::SendNotification{"Must connect at least one valid player source"}
            );
        }
    }
    apply_hover_sounds();
    apply_click_sounds();
    ImGui::SameLine();
    if (ImGui::Button("Back", ImVec2{200.f * ui_scale_, 45.f * ui_scale_})) {
        run_cancel_event();
    }
    apply_hover_sounds();
    apply_click_sounds();
    ImGui::PopFont();
    apply_touch_scroll();
    ImGui::End();
    set_next_window_fullscreen();
    ImGui::Begin(
        "PlayerSetupBackground",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoBringToFrontOnFocus
    );
    ImGui::End();
}