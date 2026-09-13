#include "gui/ui_manager.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <imgui_internal.h>

UiManager::UiManager(AssetManager& assets, UiState starting_state)
    : current_state_(starting_state) {
    font_small_ = assets.load_imgui_font(asset_paths::fonts::consolas, 12.f);
    font_medium_ = assets.load_imgui_font(asset_paths::fonts::consolas, 18.f);
    font_large_ = assets.load_imgui_font(asset_paths::fonts::consolas, 24.f);
    font_double_large_ = assets.load_imgui_font(asset_paths::fonts::consolas, 36.f);
    font_triple_large_ = assets.load_imgui_font(asset_paths::fonts::consolas, 48.f);
    font_title_ = assets.load_imgui_font(asset_paths::fonts::consolas, 128.f);
    default_style_ = ImGui::GetStyle();
}

void UiManager::update(
    WindowManager& window,
    SettingsManager& settings,
    AudioManager& audio,
    InputManager& input,
    Level* level
) {
    state_changed_this_frame_ = false;
    draw(window, settings, audio, input, level);
}

UiState UiManager::get_state() const {
    return current_state_;
}

std::string UiManager::get_state_str() const {
    switch (current_state_) {
    case UiState::main_menu:
        return "Main Menu";
    case UiState::settings:
        return "Settings";
    case UiState::player_source_setup:
        return "Player Source Setup";
    case UiState::playing:
        return "Playing";
    case UiState::paused:
        return "Paused";
    case UiState::paused_settings:
        return "Paused (Settings)";
    default:
        return "Invalid";
    }
}

void UiManager::set_state(UiState new_state) {
    if (new_state >= UiState::ui_state_count || state_changed_this_frame_ ||
        new_state == current_state_) {
        return;
    }
    UiState previous_state = current_state_;
    current_state_ = new_state;
    state_changed_this_frame_ = true;
    if (previous_state == UiState::player_source_setup) {
        // Switching off setup screen
        game_events::push(game_event_types::ShouldDetectNewPlayerSources{false});
    }
    if (new_state == UiState::player_source_setup) {
        // Switching to player source setup screen
        game_events::push(game_event_types::ShouldDetectNewPlayerSources{true});
    }
    if (previous_state == UiState::settings && did_edit_settings_) {
        game_events::push(game_event_types::SaveUserData{UserDataTypes::settings});
        did_edit_settings_ = false;
    }
    ImGuiIO& io = ImGui::GetIO();
    if (new_state == UiState::playing) {
        io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
    } else {
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    }
}

void UiManager::run_cancel_event() {
    switch (current_state_) {
    case UiState::settings:
        set_state(UiState::main_menu);
        break;
    case UiState::player_source_setup:
        set_state(UiState::main_menu);
        break;
    case UiState::playing:
        set_state(UiState::paused);
        break;
    case UiState::paused:
        set_state(UiState::playing);
        break;
    case UiState::paused_settings:
        set_state(UiState::paused);
        break;
    default:
        break;
    }
}

void UiManager::toggle_debug() {
    if (std::find(debug_visible_in_.begin(), debug_visible_in_.end(), current_state_) !=
        debug_visible_in_.end()) {
        show_debug_ = !show_debug_;
    }
}

void UiManager::pass_input_to_imgui(const game_event_types::Input& event) {
    ImGuiIO& io = ImGui::GetIO();
    ImGuiKey imgui_key = ImGuiKey_None;
    if (event.source_info.type == InputType::keyboard) {
        switch (event.verb) {
        case InputVerb::up:
            imgui_key = ImGuiKey_UpArrow;
            break;
        case InputVerb::down:
            imgui_key = ImGuiKey_DownArrow;
            break;
        case InputVerb::left:
            imgui_key = ImGuiKey_LeftArrow;
            break;
        case InputVerb::right:
            imgui_key = ImGuiKey_RightArrow;
            break;
        default:
            break;
        }
    }
    if (imgui_key != ImGuiKey_None) {
        io.AddKeyEvent(imgui_key, event.state == InputState::pressed);
    }
}

void UiManager::enable_touch_controller(Entity& entity) {
    touch_controller_ = std::make_unique<TouchController>(entity);
}

void UiManager::disable_touch_controller() {
    touch_controller_.reset();
}

int UiManager::get_free_finger_count() const {
    if (touch_controller_) {
        return touch_controller_->get_free_finger_count();
    }
    return 999;
}

void UiManager::set_scale_index(size_t scale_index) {
    const size_t max_scale_index = ui_size_presets_.size() - 1;
    if (scale_index > max_scale_index) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Clamping user preferred scale from %zu to %zu",
            scale_index,
            max_scale_index
        );
        scale_index = max_scale_index;
    }
    user_preferred_scale_ = ui_size_presets_[scale_index].scale;
    SDL_Log("User preferred UI scale set to %zu", scale_index);
}

size_t UiManager::get_scale_index() const {
    const float epsilon = 0.001f;
    for (size_t i = 0; i < ui_size_presets_.size(); i++) {
        if (std::abs(ui_size_presets_[i].scale - user_preferred_scale_) < epsilon) {
            return i;
        }
    }
    return 2;
}

void UiManager::draw(
    WindowManager& window,
    SettingsManager& settings,
    AudioManager& audio,
    InputManager& input,
    Level* level
) {
    update_active_scale(window);
    Entity* player_entity = nullptr;
    Camera* camera = nullptr;
    if (level) {
        const std::vector<Player>& players = level->get_players();
        if (!players.empty()) {
            EntityController* entity_controller = players.begin()->controller.get();
            if (entity_controller) {
                player_entity = entity_controller->get_entity();
            }
        }
        camera = level->get_camera();
    }
    if (show_debug_ &&
        std::find(debug_visible_in_.begin(), debug_visible_in_.end(), current_state_) !=
            debug_visible_in_.end()) {
        draw_debug(window, player_entity, camera, input, level);
    }
    switch (current_state_) {
    case UiState::main_menu:
        draw_main_menu(window);
        break;
    case UiState::settings:
        draw_settings(window, settings, audio, input, level);
        break;
    case UiState::player_source_setup:
        draw_player_source_setup(window, input);
        break;
    case UiState::paused:
        draw_pause_menu(window);
        break;
    case UiState::paused_settings:
        draw_settings(window, settings, audio, input, level);
        break;
    default:
        break;
    }
    if (touch_controller_ && current_state_ == UiState::playing) {
        touch_controller_->draw(window, ui_scale_);
    }
    player_source_added_this_frame_ = false;
    item_active_this_frame_ = ImGui::IsAnyItemActive();
}

void UiManager::draw_large_logo(WindowManager& window, float menu_height) {
    SDL_Rect safe_area = window.get_safe_area();
    SDL_FRect safe_area_f{
        static_cast<float>(safe_area.x),
        static_cast<float>(safe_area.y),
        static_cast<float>(safe_area.w),
        static_cast<float>(safe_area.h)
    };
    float absolute_center_x = static_cast<float>(window.get_size().x) * 0.5f;
    float ideal_padding = 5.f;
    float logo_menu_spacing = 5.f * ui_scale_;
    float total_required_height = logo_height_ + logo_menu_spacing + menu_height;
    float max_allowed_padding = safe_area_f.h - total_required_height;
    float actual_logo_top_padding =
        (max_allowed_padding < ideal_padding) ? max_allowed_padding : ideal_padding;
    if (actual_logo_top_padding < 0.f) {
        actual_logo_top_padding = 0.f;
    }
    logo_top_padding_ = actual_logo_top_padding;
    ImGui::SetNextWindowPos(
        ImVec2{absolute_center_x, safe_area_f.y + logo_top_padding_},
        ImGuiCond_Always,
        ImVec2{0.5f, 0.f}
    );
    if (ImGui::Begin(
            "Main Menu Title",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing
        )) {
        ImGui::PushFont(font_title_);
        ImGui::Text("Platformer");
        ImGui::PopFont();
        logo_height_ = ImGui::GetWindowSize().y;
    }
    ImGui::End();
}

void UiManager::fps_text(WindowManager& window) {
    float fps = ImGui::GetIO().Framerate;
    std::string text;
    if (fps >= 1000) {
        text += std::format("{:0{}.{}f}", fps, 7, 1);
    } else {
        text += std::format("{:.1f}", fps);
    }
    if (!window.is_vsync_enabled() && !window.get_fps_unlimited()) {
        text += "/" + window.target_fps_str();
    }
    text += " FPS (" + std::format("{:.3f}", 1000.f / fps) + " ms/frame)";
    ImGui::Text("%s", text.c_str());
}

void UiManager::apply_click_sounds(
    std::string_view sound_relative_path, unsigned int volume, float pitch
) {
    if (!item_active_this_frame_ && ImGui::IsItemActivated()) {
        item_active_this_frame_ = true;
        game_events::push(game_event_types::PlaySound{sound_relative_path, volume, pitch});
    }
}

void UiManager::apply_hover_sounds(
    std::string_view sound_relative_path, unsigned int volume, float pitch
) {
    // Got idea to use item ids like this from ai.
    // Before was just using a simple boolean like in applyClickSounds.
    // but that meant hover sounds could only occur if no item was hovered last frame.
    // So if i went from hovering one item in frame 1 to another in frame 2, no sound would trigger
    // Also using func below from imgui internal prevents me from having to pass const char* ids in
    // this func.
    ImGuiID current_item_id = ImGui::GetItemID();
    if (ImGui::IsItemHovered()) {
        if (last_hovered_id_ != current_item_id) {
            last_hovered_id_ = current_item_id;
            game_events::push(game_event_types::PlaySound{sound_relative_path, volume, pitch});
        }
    } else if (last_hovered_id_ == current_item_id) {
        last_hovered_id_ = 0;
    }
}

void UiManager::apply_edit_sounds(
    std::string_view sound_relative_path, unsigned int volume, float pitch
) {
    if (ImGui::IsItemEdited()) {
        game_events::push(game_event_types::PlaySound{sound_relative_path, volume, pitch});
    }
}

void UiManager::apply_touch_scroll() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.MouseSource != ImGuiMouseSource_TouchScreen) {
        return;
    }
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        ImVec2 delta = io.MouseDelta;
        ImGui::SetScrollY(ImGui::GetScrollY() - delta.y);
        ImGui::SetScrollX(ImGui::GetScrollX() - delta.x);
    }
}

void UiManager::update_active_scale(WindowManager& window) {
    SDL_Rect safe_area = window.get_safe_area();
    const float base_min_width = 480.f;
    const float base_min_height = 540.f;
    const float base_diagonal =
        std::sqrt(base_min_width * base_min_width + base_min_height * base_min_height);
    float current_diagonal =
        std::sqrt(static_cast<float>(safe_area.w * safe_area.w + safe_area.h * safe_area.h));
    float max_safe_scale = current_diagonal / base_diagonal;
    const float max_menu_width = 320.f;
    const float max_menu_height = 350.f;

    float fit_scale_w = static_cast<float>(safe_area.w) / max_menu_width;
    float fit_scale_h = static_cast<float>(safe_area.h) / max_menu_height;
    float absolute_max_scale = (fit_scale_w < fit_scale_h) ? fit_scale_w : fit_scale_h;
    if (max_safe_scale > absolute_max_scale) {
        max_safe_scale = absolute_max_scale;
    }
    if (max_safe_scale < 0.25f) {
        max_safe_scale = 0.25f;
    }
    float target_scale =
        (user_preferred_scale_ < max_safe_scale) ? user_preferred_scale_ : max_safe_scale;
    if (target_scale != ui_scale_) {
        update_style_scale(target_scale);
    }
}

void UiManager::update_style_scale(float scale) {
    ui_scale_ = scale;
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = scale;
    ImGuiStyle& style = ImGui::GetStyle();
    style = default_style_;
    style.ScaleAllSizes(scale);
    set_next_window_fullscreen();
    ImGui::Begin(
        "SettingsBackdrop",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav
    );
    ImGui::End();
}

void UiManager::set_next_window_fullscreen() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
}

void UiManager::set_next_window_safe_area(WindowManager& window) {
    SDL_Rect safe_area = window.get_safe_area();
    ImGui::SetNextWindowPos(
        ImVec2{static_cast<float>(safe_area.x), static_cast<float>(safe_area.y)}
    );
    ImGui::SetNextWindowSize(
        ImVec2{static_cast<float>(safe_area.w), static_cast<float>(safe_area.h)}
    );
}

void UiManager::set_next_window_y_only_safe_area(WindowManager& window) {
    SDL_Rect safe_area = window.get_safe_area();
    WindowVec2 window_size = window.get_size();
    ImGui::SetNextWindowPos(ImVec2{0.f, static_cast<float>(safe_area.y)});
    ImGui::SetNextWindowSize(
        ImVec2{static_cast<float>(window_size.x), static_cast<float>(safe_area.h)}
    );
}