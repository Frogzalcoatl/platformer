#include "user_data/settings_manager.h"

SettingsManager::SettingsManager(std::string_view relative_file_path) : json_(relative_file_path) {
    read_from_disk();
}

bool SettingsManager::created_new_file_on_read() const {
    return created_new_file_;
}

void SettingsManager::read_from_disk() {
    ReadFromDiskResult result = json_.read_from_disk();
    created_new_file_ = (result == ReadFromDiskResult::created_new_file);
    const rapidjson::Value& vsync_enabled = json_.get("vsyncEnabled");
    if (vsync_enabled.IsBool()) {
        active_settings_.vsync_enabled = vsync_enabled.GetBool();
    } else {
        active_settings_.vsync_enabled = default_settings_.vsync_enabled;
    }
    const rapidjson::Value& fps_unlimited = json_.get("fpsUnlimited");
    if (fps_unlimited.IsBool()) {
        active_settings_.fps_unlimited = fps_unlimited.GetBool();
    } else {
        active_settings_.fps_unlimited = default_settings_.fps_unlimited;
    }
    const rapidjson::Value& target_fps = json_.get("targetFps");
    if (target_fps.IsUint()) {
        active_settings_.target_fps = target_fps.GetUint();
    } else {
        active_settings_.target_fps = default_settings_.target_fps;
    }
    const rapidjson::Value& ui_scale = json_.get("uiScale");
    if (ui_scale.IsUint64()) {
        active_settings_.ui_scale = ui_scale.GetUint64();
    } else {
        active_settings_.ui_scale = default_settings_.ui_scale;
    }
    const rapidjson::Value& master_volume = json_.get("masterVolume");
    if (master_volume.IsUint()) {
        active_settings_.master_volume = master_volume.GetUint();
    } else {
        active_settings_.master_volume = default_settings_.master_volume;
    }
    const rapidjson::Value& sounds_volume = json_.get("soundsVolume");
    if (sounds_volume.IsUint()) {
        active_settings_.sounds_volume = sounds_volume.GetUint();
    } else {
        active_settings_.sounds_volume = default_settings_.sounds_volume;
    }
    const rapidjson::Value& music_volume = json_.get("musicVolume");
    if (music_volume.IsUint()) {
        active_settings_.music_volume = music_volume.GetUint();
    } else {
        active_settings_.music_volume = default_settings_.music_volume;
    }
}

bool SettingsManager::save_to_disk() {
    json_.set("vsyncEnabled", active_settings_.vsync_enabled);
    json_.set("fpsUnlimited", active_settings_.fps_unlimited);
    json_.set("targetFps", active_settings_.target_fps);
    json_.set("uiScale", active_settings_.ui_scale);
    json_.set("masterVolume", active_settings_.master_volume);
    json_.set("soundsVolume", active_settings_.sounds_volume);
    json_.set("musicVolume", active_settings_.music_volume);
    return json_.save_to_disk();
}