#include "user_data/json_manager.h"
#include <filesystem>

JsonManager::JsonManager(std::string_view relative_file_path)
    : relative_file_path_(relative_file_path) {
    doc_.SetObject();
    const char* pref_path_str = SDL_GetPrefPath("Frogzalcoatl", "Platformer");
    if (!pref_path_str) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL_GetPrefPath unsuccessful in SettingsManager: %s",
            SDL_GetError()
        );
    }
    std::filesystem::path pref_path{pref_path_str};
    file_path_ = pref_path / relative_file_path;
    file_path_str_ = file_path_.string();
}

FileExistsResult JsonManager::file_exists() {
    SDL_PathInfo path_info{};
    if (!SDL_GetPathInfo(file_path_str_.c_str(), &path_info)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to access file \"%s\": %s",
            file_path_str_.c_str(),
            SDL_GetError()
        );
        return FileExistsResult::does_not_exist;
    }
    if (path_info.type != SDL_PATHTYPE_FILE) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to access file \"%s\": Path currently holds non file type",
            file_path_str_.c_str()
        );
        return FileExistsResult::holds_non_file_type;
    }
    return FileExistsResult::success;
}

bool JsonManager::create_file() {
    SDL_Log("Creating file \"%s\"...", file_path_str_.c_str());
    SDL_IOStream* io = SDL_IOFromFile(file_path_str_.c_str(), "w");
    if (!io) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return false;
    }
    if (!SDL_CloseIO(io)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Unable to close file creation SDL io: %s", SDL_GetError()
        );
        return false;
    }
    return true;
}

bool JsonManager::save_to_disk() {
    FileExistsResult exists_result = file_exists();
    if (exists_result == FileExistsResult::holds_non_file_type) {
        return false;
    } else if (exists_result == FileExistsResult::does_not_exist) {
        if (!create_file()) {
            return false;
        }
    }
    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    doc_.Accept(writer);
    SDL_IOStream* io = SDL_IOFromFile(file_path_str_.c_str(), "w");
    if (!io) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to write json to file \"%s\": %s",
            file_path_str_.c_str(),
            SDL_GetError()
        );
        return false;
    }
    size_t string_buffer_size = string_buffer.GetSize();
    size_t write_size = SDL_WriteIO(io, string_buffer.GetString(), string_buffer_size);
    SDL_CloseIO(io);
    if (string_buffer_size != write_size) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to write json to file \"%s\": %s",
            file_path_str_.c_str(),
            SDL_GetError()
        );
        return false;
    }
    SDL_Log("Saved json to file \"%s\"", relative_file_path_.c_str());
    return true;
}

ReadFromDiskResult JsonManager::read_from_disk() {
    FileExistsResult exists_result = file_exists();
    bool created_new_file = false;
    if (exists_result == FileExistsResult::holds_non_file_type) {
        return ReadFromDiskResult::failure;
    } else if (exists_result == FileExistsResult::does_not_exist) {
        if (!create_file()) {
            return ReadFromDiskResult::failure;
        }
        created_new_file = true;
    }
    SDL_IOStream* io = SDL_IOFromFile(file_path_.string().c_str(), "r");
    if (!io) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to create SDL IO to read file: \"%s\"",
            file_path_str_.c_str()
        );
        return ReadFromDiskResult::failure;
    }
    Sint64 file_size = SDL_GetIOSize(io);
    if (file_size < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to read file size: %s", SDL_GetError());
        return ReadFromDiskResult::failure;
    }
    // Second arg is the char each index of the str will be initialized to
    std::string file_buffer(static_cast<size_t>(file_size), '\0');
    size_t bytes_read = SDL_ReadIO(io, file_buffer.data(), static_cast<size_t>(file_size));
    SDL_CloseIO(io);
    if (bytes_read != static_cast<size_t>(file_size)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to read file \"%s\": %s",
            file_path_str_.c_str(),
            SDL_GetError()
        );
        return ReadFromDiskResult::failure;
    }
    doc_.Parse(file_buffer.data());
    if (doc_.HasParseError()) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Parse error in \"%s\", clearing...",
            relative_file_path_.c_str()
        );
        doc_.SetObject();
        return save_to_disk() ? ReadFromDiskResult::created_new_file : ReadFromDiskResult::failure;
    }
    if (!doc_.IsObject()) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "\"%s\" should be an Object, clearing...",
            relative_file_path_.c_str()
        );
        doc_.SetObject();
        return save_to_disk() ? ReadFromDiskResult::created_new_file : ReadFromDiskResult::failure;
    }
    SDL_Log("Read json from file \"%s\"", relative_file_path_.c_str());
    return created_new_file ? ReadFromDiskResult::created_new_file
                            : ReadFromDiskResult::read_from_file;
}

void JsonManager::set(std::string_view key_view, rapidjson::Value& value) {
    rapidjson::Value key(
        key_view.data(), static_cast<rapidjson::SizeType>(key_view.length()), doc_.GetAllocator()
    );
    if (doc_.HasMember(key)) {
        doc_.RemoveMember(key);
    }
    doc_.AddMember(key, value, doc_.GetAllocator());
}

const rapidjson::Value& JsonManager::get(std::string_view key_view) {
    if (!doc_.IsObject()) {
        return null_value_;
    }
    rapidjson::Value key(
        key_view.data(), static_cast<rapidjson::SizeType>(key_view.length()), doc_.GetAllocator()
    );
    auto it = doc_.FindMember(key);
    if (it != doc_.MemberEnd()) {
        return it->value;
    } else {
        return null_value_;
    }
}