#pragma once
#include "gui/touch_controller.h"
#include "platformer/game_events.h"
#include "platformer/level.h"
#include "system/audio_manager.h"
#include "system/input_manager.h"
#include "system/window_manager.h"
#include "user_data/settings_manager.h"
#include <array>
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

struct UiSizePreset {
    float scale;
    const char* name;
};

class UiManager {
  public:
    UiManager(AssetManager& assets, UiState starting_state = UiState::main_menu);

    void update(
        WindowManager& window,
        SettingsManager& settings,
        AudioManager& audio,
        InputManager& input,
        Level* level
    );

    UiState get_state() const;
    std::string get_state_str() const;
    void set_state(UiState state);
    void run_cancel_event();
    void toggle_debug();

    void pass_input_to_imgui(const game_event_types::Input& event);
    void enable_touch_controller(Entity& entity);
    void disable_touch_controller();
    int get_free_finger_count() const;

    void set_scale_index(size_t scale_index);
    size_t get_scale_index() const;
    float get_actual_scale() const {
        return ui_scale_;
    }

    void set_player_source_added_this_frame(bool value) {
        player_source_added_this_frame_ = value;
    }
    bool is_player_source_added_this_frame() const {
        return player_source_added_this_frame_;
    }

  private:
    void draw(
        WindowManager& window,
        SettingsManager& settings,
        AudioManager& audio,
        InputManager& input,
        Level* level
    );
    void draw_main_menu(WindowManager& window);
    void draw_settings(
        WindowManager& window,
        SettingsManager& settings,
        AudioManager& audio,
        InputManager& input,
        Level* level
    );
    void draw_player_source_setup(WindowManager& window, InputManager& input);
    void draw_pause_menu(WindowManager& window);
    void draw_debug(
        WindowManager& window, Entity* player, Camera* camera, InputManager& input, Level* level
    );
    void draw_large_logo(WindowManager& window, float menu_height);
    void fps_text(WindowManager& window);

    void apply_click_sounds(
        std::string_view sound_relative_path = asset_paths::sounds::click,
        unsigned int volume = 100,
        float pitch = 1.f
    );
    void apply_hover_sounds(
        std::string_view sound_relative_path = asset_paths::sounds::hover,
        unsigned int volume = 100,
        float pitch = 1.f
    );
    void apply_edit_sounds(
        std::string_view sound_relative_path = asset_paths::sounds::edit,
        unsigned int volume = 100,
        float pitch = 1.f
    );
    void apply_touch_scroll();
    void update_active_scale(WindowManager& window);
    void update_style_scale(float scale);
    void set_next_window_fullscreen();
    void set_next_window_safe_area(WindowManager& window);
    void set_next_window_y_only_safe_area(WindowManager& window);

    UiState current_state_ = UiState::main_menu;
    bool state_changed_this_frame_ = false;
    bool player_source_added_this_frame_ = false;

    ImGuiID last_hovered_id_ = 0;
    bool item_active_this_frame_ = false;
    bool did_edit_settings_ = false;

    float ui_scale_ = 1.f;
    float user_preferred_scale_ = 1.5f;
    ImGuiStyle default_style_;

    float logo_height_ = 0.f;
    float logo_top_padding_ = 0.f;

    ImFont* font_small_ = nullptr;
    ImFont* font_medium_ = nullptr;
    ImFont* font_large_ = nullptr;
    ImFont* font_double_large_ = nullptr;
    ImFont* font_triple_large_ = nullptr;
    ImFont* font_title_ = nullptr;

    const int max_volume_ = 100;

    std::unique_ptr<TouchController> touch_controller_ = nullptr;

    bool show_debug_ = false;
    const std::vector<UiState> debug_visible_in_ = {
        UiState::main_menu, UiState::paused, UiState::playing
    };

    const std::array<UiSizePreset, 6> ui_size_presets_ = {
        {{0.5f, "Extra Small"},
         {1.0f, "Small"},
         {1.5f, "Normal"},
         {2.0f, "Large"},
         {2.5f, "Extra Large"},
         {3.0f, "Ginormous"}}
    };
};