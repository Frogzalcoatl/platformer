#pragma once
#include <SDL3/SDL.h>
#include <filesystem>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

enum class FileExistsResult : uint8_t {
    Success,
    DoesNotExist,
    HoldsNonFileType
};

class JsonManager {
  protected:
    std::filesystem::path filePath;
    std::string filePathStr;
    std::string relativeFilePath;
    rapidjson::Document doc;
    const rapidjson::Value nullValue;

    bool readFromDisk();

    FileExistsResult fileExists();

    bool createFile();

  public:
    JsonManager(std::string_view relativeFilePath);

    bool saveToDisk();

    // Template helper (Idea from AI)
    template <typename T> void set(std::string_view key, T value) {
        rapidjson::Value rJsonValue{value};
        set(key, rJsonValue);
    }

    void set(std::string_view key, rapidjson::Value& value);

    const rapidjson::Value& get(std::string_view key);
};