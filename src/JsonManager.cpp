#include "JsonManager.hpp"
#include <filesystem>

JsonManager::JsonManager(std::string_view relativeFilePath) : relativeFilePath(relativeFilePath) {
    doc.SetObject();
    // SDL_GetPrefPath is specifically for user data files
    const char* prefPathStr = SDL_GetPrefPath("Frogzalcoatl", "Platformer");
    if (!prefPathStr) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL_GetPrefPath unsuccessful in SettingsManager: %s",
            SDL_GetError()
        );
    }
    std::filesystem::path basePath{prefPathStr};
    filePath = basePath / relativeFilePath;
    filePathStr = filePath.string();
}

bool JsonManager::doesFileExist() {
    SDL_PathInfo pathInfo{};
    if (!SDL_GetPathInfo(filePathStr.c_str(), &pathInfo)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to access file \"%s\": %s",
            relativeFilePath.c_str(),
            SDL_GetError()
        );
        return false;
    }
    if (pathInfo.type != SDL_PATHTYPE_FILE) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to access file \"%s\": Path currently holds non file type",
            relativeFilePath.c_str()
        );
        return false;
    }
    SDL_Log("File \"%s\" exists!", relativeFilePath.c_str());
    return true;
}

bool JsonManager::createFile() {
    SDL_Log("Creating file \"%s\"...", relativeFilePath.c_str());
    SDL_IOStream* io = SDL_IOFromFile(filePathStr.c_str(), "w");
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

bool JsonManager::saveToDisk() {
    SDL_Log("Saving \"%s\" to disk...", relativeFilePath.c_str());
    if (!doesFileExist()) {
        if (!createFile()) {
            return false;
        }
    }
    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
    doc.Accept(writer);
    SDL_IOStream* io = SDL_IOFromFile(filePathStr.c_str(), "w");
    if (!io) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to write json to file \"%s\": %s",
            relativeFilePath.c_str(),
            SDL_GetError()
        );
        return false;
    }
    size_t stringBufferSize = stringBuffer.GetSize();
    size_t writeSize = SDL_WriteIO(io, stringBuffer.GetString(), stringBufferSize);
    SDL_CloseIO(io);
    if (stringBufferSize != writeSize) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to write json to file \"%s\": %s",
            relativeFilePath.c_str(),
            SDL_GetError()
        );
        return false;
    }
    SDL_Log("Wrote json to file \"%s\"", relativeFilePath.c_str());
    return true;
}

bool JsonManager::readFromDisk() {
    SDL_Log("Reading \"%s\" from disk...", relativeFilePath.c_str());
    if (!doesFileExist()) {
        if (!createFile()) {
            return false;
        }
    }
    SDL_IOStream* io = SDL_IOFromFile(filePath.string().c_str(), "r");
    if (!io) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to create SDL IO to read file: \"%s\"",
            relativeFilePath.c_str()
        );
        return false;
    }
    Sint64 fileSize = SDL_GetIOSize(io);
    if (fileSize < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to read file size: %s", SDL_GetError());
        return false;
    }
    // Second arg is the char each index of the str will be initialized to
    std::string fileBuffer(static_cast<size_t>(fileSize), '\0');
    size_t bytesRead = SDL_ReadIO(io, fileBuffer.data(), static_cast<size_t>(fileSize));
    SDL_CloseIO(io);
    if (bytesRead != static_cast<size_t>(fileSize)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to read file \"%s\": %s",
            relativeFilePath.c_str(),
            SDL_GetError()
        );
        return false;
    }
    doc.Parse(fileBuffer.data());
    if (doc.HasParseError()) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Parse error in \"%s\", clearing...",
            relativeFilePath.c_str()
        );
        doc.SetObject();
        return saveToDisk();
    }
    if (!doc.IsObject()) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "\"%s\" should be an Object, clearing...",
            relativeFilePath.c_str()
        );
        doc.SetObject();
        return saveToDisk();
    }
    return true;
}

void JsonManager::set(std::string_view keyView, rapidjson::Value& value) {
    rapidjson::Value key(
        keyView.data(), static_cast<rapidjson::SizeType>(keyView.length()), doc.GetAllocator()
    );
    if (doc.HasMember(key)) {
        doc.RemoveMember(key);
    }
    doc.AddMember(key, value, doc.GetAllocator());
}

const rapidjson::Value& JsonManager::get(std::string_view keyView) {
    if (!doc.IsObject()) {
        return nullValue;
    }
    rapidjson::Value key(
        keyView.data(), static_cast<rapidjson::SizeType>(keyView.length()), doc.GetAllocator()
    );
    auto it = doc.FindMember(key);
    if (it != doc.MemberEnd()) {
        return it->value;
    } else {
        return nullValue;
    }
}