#pragma once
#include <SDL3/SDL.h>
#include <filesystem>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

class JsonManager {
  protected:
    std::filesystem::path filePath;
    std::string filePathStr;
    std::string relativeFilePath;
    rapidjson::Document doc;
    const rapidjson::Value nullValue;

    bool readFromDisk();

    bool doesFileExist();

    bool createFile();

  public:
    JsonManager(std::string_view relativeFilePath);

    bool saveToDisk();

    // Template helper (Idea from AI)
    template <typename T> void set(std::string_view key, T value) {
        rapidjson::Value rValue{value};
        set(key, rValue);
    }

    void set(std::string_view key, rapidjson::Value& value);

    const rapidjson::Value& get(std::string_view key);
};