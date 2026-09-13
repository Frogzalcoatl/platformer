#pragma once
#include <SDL3/SDL.h>
#include <filesystem>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

enum class FileExistsResult : uint8_t {
    success,
    does_not_exist,
    holds_non_file_type
};

enum class ReadFromDiskResult : uint8_t {
    read_from_file,
    created_new_file,
    failure
};

class JsonManager {
  private:
    std::filesystem::path file_path_;
    std::string file_path_str_;
    std::string relative_file_path_;
    rapidjson::Document doc_;
    const rapidjson::Value null_value_;

    FileExistsResult file_exists();
    bool create_file();

  public:
    JsonManager(std::string_view relative_file_path);

    ReadFromDiskResult read_from_disk();

    bool save_to_disk();

    // Template helper (Idea from AI)
    template <typename T> void set(std::string_view key, T value) {
        rapidjson::Value rjson_value{value};
        set(key, rjson_value);
    }

    void set(std::string_view key, rapidjson::Value& value);

    const rapidjson::Value& get(std::string_view key);
};