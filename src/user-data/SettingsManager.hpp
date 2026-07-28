#pragma once
#include "user-data/JsonManager.hpp"
#include <string>
#include <unordered_map>

struct Settings {
    bool vsyncEnabled = true;
    bool fpsUnlimited = false;
    unsigned int targetFps = 120;
    uint64_t uiScale = 2;
    unsigned int masterVolume = 100;
    unsigned int soundsVolume = 100;
    unsigned int musicVolume = 50;
};

class SettingsManager {
  private:
    JsonManager json;
    const Settings defaultSettings;
    Settings activeSettings;
    bool createdNewFile;

    void readFromDisk();

  public:
    SettingsManager(std::string_view relativeFilePath);

    bool saveToDisk();

    const Settings& get() const {
        return activeSettings;
    }

    const Settings& getDefault() const {
        return defaultSettings;
    }

    bool createdNewFileOnRead() const;

    void setVsyncEnabled(bool val) {
        activeSettings.vsyncEnabled = val;
    }
    void setFpsUnlimited(bool val) {
        activeSettings.fpsUnlimited = val;
    }
    void setTargetFps(unsigned int val) {
        activeSettings.targetFps = val;
    }
    void setUiScale(uint64_t val) {
        activeSettings.uiScale = val;
    }
    void setMasterVolume(unsigned int val) {
        activeSettings.masterVolume = val;
    }
    void setSoundsVolume(unsigned int val) {
        activeSettings.soundsVolume = val;
    }
    void setMusicVolume(unsigned int val) {
        activeSettings.musicVolume = val;
    }
};