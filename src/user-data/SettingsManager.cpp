#include "user-data/SettingsManager.hpp"

SettingsManager::SettingsManager(std::string_view relativeFilePath)
    : JsonManager(relativeFilePath) {
    readFromDisk();
}

bool SettingsManager::createdNewFileOnRead() const {
    return createdNewFile;
}

void SettingsManager::readFromDisk() {
    ReadFromDiskResult result = JsonManager::readFromDisk();
    createdNewFile = (result == ReadFromDiskResult::CreatedNewFile);
    const rapidjson::Value& vsyncEnabled = JsonManager::get("vsyncEnabled");
    if (vsyncEnabled.IsBool()) {
        activeSettings.vsyncEnabled = vsyncEnabled.GetBool();
    } else {
        activeSettings.vsyncEnabled = defaultSettings.vsyncEnabled;
    }
    const rapidjson::Value& fpsUnlimited = JsonManager::get("fpsUnlimited");
    if (fpsUnlimited.IsBool()) {
        activeSettings.fpsUnlimited = fpsUnlimited.GetBool();
    } else {
        activeSettings.fpsUnlimited = defaultSettings.fpsUnlimited;
    }
    const rapidjson::Value& targetFps = JsonManager::get("targetFps");
    if (targetFps.IsUint()) {
        activeSettings.targetFps = targetFps.GetUint();
    } else {
        activeSettings.targetFps = defaultSettings.targetFps;
    }
    const rapidjson::Value& uiScale = JsonManager::get("uiScale");
    if (uiScale.IsUint64()) {
        activeSettings.uiScale = uiScale.GetUint64();
    } else {
        activeSettings.uiScale = defaultSettings.uiScale;
    }
    const rapidjson::Value& masterVolume = JsonManager::get("masterVolume");
    if (masterVolume.IsUint()) {
        activeSettings.masterVolume = masterVolume.GetUint();
    } else {
        activeSettings.masterVolume = defaultSettings.masterVolume;
    }
    const rapidjson::Value& soundsVolume = JsonManager::get("soundsVolume");
    if (soundsVolume.IsUint()) {
        activeSettings.soundsVolume = soundsVolume.GetUint();
    } else {
        activeSettings.soundsVolume = defaultSettings.soundsVolume;
    }
    const rapidjson::Value& musicVolume = JsonManager::get("musicVolume");
    if (musicVolume.IsUint()) {
        activeSettings.musicVolume = musicVolume.GetUint();
    } else {
        activeSettings.musicVolume = defaultSettings.musicVolume;
    }
}

bool SettingsManager::saveToDisk() {
    set("vsyncEnabled", activeSettings.vsyncEnabled);
    set("fpsUnlimited", activeSettings.fpsUnlimited);
    set("targetFps", activeSettings.targetFps);
    set("uiScale", activeSettings.uiScale);
    set("masterVolume", activeSettings.masterVolume);
    set("soundsVolume", activeSettings.soundsVolume);
    set("musicVolume", activeSettings.musicVolume);
    return JsonManager::saveToDisk();
}