#include "gui/ui_manager.h"
#include <limits>

void UiManager::draw_settings(
    WindowManager& window,
    SettingsManager& settings,
    AudioManager& audio,
    InputManager& input,
    Level* level
) {
    set_next_window_safe_area(window);
    const ImVec2 vertical_spacing_dummy{0.f, 10.f * ui_scale_};
    const ImVec2 horizontal_spacing_dummy{10.f * ui_scale_, 0.f};
    ImVec2 reset_button_size{100 * ui_scale_, 30 * ui_scale_};
    if (ImGui::Begin(
            "Settings",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
        )) {
        ImGui::PushFont(font_large_);
        if (ImGui::Button("Back")) {
            run_cancel_event();
        }
        apply_hover_sounds();
        apply_click_sounds();
        ImGui::SameLine();
        ImGui::Dummy(horizontal_spacing_dummy);
        ImGui::SameLine();
        if (ImGui::BeginTabBar("SettingsTabBar")) {
            ImGuiTabItemFlags display_tab_flags = ImGuiTabItemFlags_None;
            if (ImGui::IsWindowAppearing()) {
                display_tab_flags |= ImGuiTabItemFlags_SetSelected;
            }
            if (ImGui::BeginTabItem("Display", nullptr, display_tab_flags)) {
                apply_hover_sounds();
                apply_click_sounds();
                ImGui::PushFont(font_double_large_);
                ImGui::Text("Display");
                ImGui::PopFont();
                ImGui::Dummy(vertical_spacing_dummy);
                fps_text(window);
                ImGui::Dummy(vertical_spacing_dummy);
                bool vsync = window.is_vsync_enabled();
#ifndef SDL_PLATFORM_ANDROID
                if (ImGui::Checkbox("VSync", &vsync)) {
                    window.set_vsync(vsync);
                    settings.set_vsync_enabled(vsync);
                    settings.set_fps_unlimited(window.get_fps_unlimited());
                    did_edit_settings_ = true;
                }
                apply_hover_sounds();
                apply_click_sounds();
                ImGui::Dummy(vertical_spacing_dummy);
#endif
                bool fps_unlimited = window.get_fps_unlimited();
                if (ImGui::Checkbox("FPS Unlimited", &fps_unlimited)) {
                    window.set_fps_unlimited(fps_unlimited);
                    settings.set_fps_unlimited(fps_unlimited);
                    settings.set_vsync_enabled(window.is_vsync_enabled());
                    did_edit_settings_ = true;
                }
                apply_hover_sounds();
                apply_click_sounds();
                if (!vsync && !fps_unlimited) {
                    ImGui::Dummy(vertical_spacing_dummy);
                    static int temp_fps = static_cast<int>(window.get_target_fps());
                    ImGui::SliderInt(
                        "Target FPS", &temp_fps, 10, 300, "%d", ImGuiSliderFlags_NoInput
                    );
                    apply_click_sounds();
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        window.set_target_fps(static_cast<Uint64>(temp_fps));
                        settings.set_target_fps(static_cast<unsigned int>(temp_fps));
                        did_edit_settings_ = true;
                    }
                    if (!ImGui::IsItemActive()) {
                        temp_fps = static_cast<int>(window.get_target_fps());
                    }
                }
                ImGui::Dummy(vertical_spacing_dummy);
                int active_index = 0;
                float min_diff = std::numeric_limits<float>::max();
                for (size_t i = 0; i < ui_size_presets_.size(); i++) {
                    float diff = std::abs(ui_size_presets_[i].scale - user_preferred_scale_);
                    if (diff < min_diff) {
                        min_diff = diff;
                        active_index = static_cast<int>(i);
                    }
                }
                static int temp_index = active_index;
                if (ImGui::Button("Reset##ResetUIScale", reset_button_size)) {
                    const Settings& default_settings = settings.get_default();
                    user_preferred_scale_ = ui_size_presets_[default_settings.ui_scale].scale;
                    temp_index = static_cast<int>(default_settings.ui_scale);
                    settings.set_ui_scale(default_settings.ui_scale);
                    did_edit_settings_ = true;
                }
                apply_hover_sounds();
                apply_click_sounds();
                ImGui::SameLine();
                ImGui::Dummy(horizontal_spacing_dummy);
                ImGui::SameLine();
                ImGui::SliderInt(
                    "UI Scale",
                    &temp_index,
                    0,
                    static_cast<int>(ui_size_presets_.size() - 1),
                    ui_size_presets_[static_cast<size_t>(temp_index)].name
                );
                apply_click_sounds();
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    user_preferred_scale_ = ui_size_presets_[static_cast<size_t>(temp_index)].scale;
                    active_index = temp_index;
                    settings.set_ui_scale(static_cast<unsigned int>(temp_index));
                    did_edit_settings_ = true;
                }
                if (!ImGui::IsItemActive()) {
                    temp_index = active_index;
                }
                if (level) {
                    ImGui::Dummy(vertical_spacing_dummy);
                    ImGui::Checkbox("Show Hitboxes", &level->show_hitboxes);
                    apply_hover_sounds();
                    apply_click_sounds();
                }
                ImGui::EndTabItem();
            } else {
                apply_hover_sounds();
                apply_click_sounds();
            }
            if (ImGui::BeginTabItem("Audio")) {
                apply_hover_sounds();
                apply_click_sounds();
                ImGui::PushFont(font_double_large_);
                ImGui::Text("Audio");
                ImGui::PopFont();
                int master_volume = static_cast<int>(audio.get_volume(AudioCategory::master));
                int sound_volume = static_cast<int>(audio.get_volume(AudioCategory::sounds));
                int music_volume = static_cast<int>(audio.get_volume(AudioCategory::music));
                float pitch = audio.get_music_pitch();
                ImGui::Dummy(vertical_spacing_dummy);
                if (ImGui::Button("Reset##ResetMaster", reset_button_size)) {
                    audio.set_volume(AudioCategory::master, 100);
                    settings.set_master_volume(100);
                    did_edit_settings_ = true;
                }
                apply_hover_sounds();
                apply_click_sounds();
                ImGui::SameLine();
                ImGui::Dummy(horizontal_spacing_dummy);
                ImGui::SameLine();
                if (ImGui::SliderInt(
                        "Master", &master_volume, 0, max_volume_, "%d", ImGuiSliderFlags_NoInput
                    )) {
                    audio.set_volume(
                        AudioCategory::master, static_cast<unsigned int>(master_volume)
                    );
                    settings.set_master_volume(static_cast<unsigned int>(master_volume));
                    did_edit_settings_ = true;
                }
                apply_click_sounds();
                ImGui::Dummy(vertical_spacing_dummy);
                if (ImGui::Button("Reset##ResetSounds", reset_button_size)) {
                    audio.set_volume(AudioCategory::sounds, 100);
                    settings.set_sounds_volume(100);
                    did_edit_settings_ = true;
                }
                apply_hover_sounds();
                apply_click_sounds();
                ImGui::SameLine();
                ImGui::Dummy(horizontal_spacing_dummy);
                ImGui::SameLine();
                if (ImGui::SliderInt(
                        "Sounds", &sound_volume, 0, max_volume_, "%d", ImGuiSliderFlags_NoInput
                    )) {
                    audio.set_volume(
                        AudioCategory::sounds, static_cast<unsigned int>(sound_volume)
                    );
                    settings.set_sounds_volume(static_cast<unsigned int>(sound_volume));
                    did_edit_settings_ = true;
                }
                apply_click_sounds();
                ImGui::Dummy(vertical_spacing_dummy);
                if (ImGui::Button("Reset##ResetMusic", reset_button_size)) {
                    audio.set_volume(AudioCategory::music, 100);
                    settings.set_music_volume(100);
                    did_edit_settings_ = true;
                }
                ImGui::SameLine();
                ImGui::Dummy(horizontal_spacing_dummy);
                ImGui::SameLine();
                if (ImGui::SliderInt(
                        "Music", &music_volume, 0, max_volume_, "%d", ImGuiSliderFlags_NoInput
                    )) {
                    audio.set_volume(AudioCategory::music, static_cast<unsigned int>(music_volume));
                    settings.set_music_volume(static_cast<unsigned int>(music_volume));
                    did_edit_settings_ = true;
                }
                apply_click_sounds();
                ImGui::Dummy(vertical_spacing_dummy);
                if (ImGui::Button("Reset##ResetMusicPitch", reset_button_size)) {
                    audio.set_music_pitch(1.f);
                    did_edit_settings_ = true;
                }
                ImGui::SameLine();
                ImGui::Dummy(horizontal_spacing_dummy);
                ImGui::SameLine();
                if (ImGui::SliderFloat(
                        "Music Pitch", &pitch, 0.5f, 1.5f, "%.2f", ImGuiSliderFlags_NoInput
                    )) {
                    audio.set_music_pitch(pitch);
                    did_edit_settings_ = true;
                }
                apply_click_sounds();
                ImGui::Dummy(vertical_spacing_dummy);
                ImGui::Text(
                    "Current Music: %s %s",
                    audio.get_current_music_name().c_str(),
                    audio.is_music_looping() ? "(Looping)" : ""
                );
                ImGui::Text("Timestamp: %s", audio.formatted_music_time().c_str());
                ImGui::Dummy(vertical_spacing_dummy);
                if (ImGui::Button("Play Random Music")) {
                    audio.clear_current_music();
                }
                apply_click_sounds();
                apply_hover_sounds();
                ImGui::EndTabItem();
            } else {
                apply_hover_sounds();
                apply_click_sounds();
            }
            if (ImGui::BeginTabItem("Controls")) {
                apply_click_sounds();
                apply_hover_sounds();
                ImGui::PushFont(font_double_large_);
                ImGui::Text("Controls (Unfinished)");
                ImGui::PopFont();
                const ScancodeBindings& scancode_bindings = input.get_scancode_bindings();
                for (size_t i = 0; i < static_cast<size_t>(InputVerb::verb_count); i++) {
                    ImGui::Dummy(ImVec2{0.f, 25.f * ui_scale_});
                    std::string current_verb = input_verb_to_string(static_cast<InputVerb>(i));
                    ImGui::Text("%s: ", current_verb.c_str());
                    for (size_t j = 0; j < max_binds_per_verb; j++) {
                        std::string current = SDL_GetScancodeName(scancode_bindings[i][j].scancode);
                        current += "##" + current_verb + "Index" + std::to_string(j);
                        ImGui::Button(current.c_str(), ImVec2{200.f * ui_scale_, 50.f * ui_scale_});
                        apply_click_sounds();
                        apply_hover_sounds();
                        ImGui::SameLine();
                        ImGui::Dummy(ImVec2{10.f * ui_scale_, 0.f});
                        ImGui::SameLine();
                    }
                    ImGui::NewLine();
                }
                ImGui::EndTabItem();
            } else {
                apply_hover_sounds();
                apply_click_sounds();
            }
            ImGui::EndTabBar();
        }
        apply_touch_scroll();
        ImGui::PopFont();
    }
    ImGui::End();
    set_next_window_fullscreen();
    ImGui::Begin(
        "SettingsBackground",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoBringToFrontOnFocus
    );
    ImGui::End();
}