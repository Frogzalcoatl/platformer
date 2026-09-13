#pragma once
#include "user_data/json_manager.h"
#include <cstdint>

struct Settings {
    bool vsync_enabled = true;
    bool fps_unlimited = false;
    unsigned int target_fps = 120;
    uint64_t ui_scale = 2;
    unsigned int master_volume = 100;
    unsigned int sounds_volume = 100;
    unsigned int music_volume = 50;
};

class SettingsManager {
  private:
    JsonManager json_;
    const Settings default_settings_;
    Settings active_settings_;
    bool created_new_file_ = false;

    void read_from_disk();

  public:
    SettingsManager(std::string_view relative_file_path);

    bool save_to_disk();

    const Settings& get() const {
        return active_settings_;
    }

    const Settings& get_default() const {
        return default_settings_;
    }

    bool created_new_file_on_read() const;

    void set_vsync_enabled(bool val) {
        active_settings_.vsync_enabled = val;
    }
    void set_fps_unlimited(bool val) {
        active_settings_.fps_unlimited = val;
    }
    void set_target_fps(unsigned int val) {
        active_settings_.target_fps = val;
    }
    void set_ui_scale(uint64_t val) {
        active_settings_.ui_scale = val;
    }
    void set_master_volume(unsigned int val) {
        active_settings_.master_volume = val;
    }
    void set_sounds_volume(unsigned int val) {
        active_settings_.sounds_volume = val;
    }
    void set_music_volume(unsigned int val) {
        active_settings_.music_volume = val;
    }
};