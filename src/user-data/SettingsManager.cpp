#include "user-data/SettingsManager.hpp"

SettingsManager::SettingsManager(std::string_view relativeFilePath) : json(relativeFilePath) {
    readFromDisk();
}

bool SettingsManager::createdNewFileOnRead() const {
    return createdNewFile;
}

void SettingsManager::readFromDisk() {
    ReadFromDiskResult result = json.readFromDisk();
    createdNewFile = (result == ReadFromDiskResult::CreatedNewFile);
    const rapidjson::Value& vsyncEnabled = json.get("vsyncEnabled");
    if (vsyncEnabled.IsBool()) {
        activeSettings.vsyncEnabled = vsyncEnabled.GetBool();
    } else {
        activeSettings.vsyncEnabled = defaultSettings.vsyncEnabled;
    }
    const rapidjson::Value& fpsUnlimited = json.get("fpsUnlimited");
    if (fpsUnlimited.IsBool()) {
        activeSettings.fpsUnlimited = fpsUnlimited.GetBool();
    } else {
        activeSettings.fpsUnlimited = defaultSettings.fpsUnlimited;
    }
    const rapidjson::Value& targetFps = json.get("targetFps");
    if (targetFps.IsUint()) {
        activeSettings.targetFps = targetFps.GetUint();
    } else {
        activeSettings.targetFps = defaultSettings.targetFps;
    }
    const rapidjson::Value& uiScale = json.get("uiScale");
    if (uiScale.IsUint64()) {
        activeSettings.uiScale = uiScale.GetUint64();
    } else {
        activeSettings.uiScale = defaultSettings.uiScale;
    }
    const rapidjson::Value& masterVolume = json.get("masterVolume");
    if (masterVolume.IsUint()) {
        activeSettings.masterVolume = masterVolume.GetUint();
    } else {
        activeSettings.masterVolume = defaultSettings.masterVolume;
    }
    const rapidjson::Value& soundsVolume = json.get("soundsVolume");
    if (soundsVolume.IsUint()) {
        activeSettings.soundsVolume = soundsVolume.GetUint();
    } else {
        activeSettings.soundsVolume = defaultSettings.soundsVolume;
    }
    const rapidjson::Value& musicVolume = json.get("musicVolume");
    if (musicVolume.IsUint()) {
        activeSettings.musicVolume = musicVolume.GetUint();
    } else {
        activeSettings.musicVolume = defaultSettings.musicVolume;
    }
}

bool SettingsManager::saveToDisk() {
    json.set("vsyncEnabled", activeSettings.vsyncEnabled);
    json.set("fpsUnlimited", activeSettings.fpsUnlimited);
    json.set("targetFps", activeSettings.targetFps);
    json.set("uiScale", activeSettings.uiScale);
    json.set("masterVolume", activeSettings.masterVolume);
    json.set("soundsVolume", activeSettings.soundsVolume);
    json.set("musicVolume", activeSettings.musicVolume);
    return json.saveToDisk();
}