#include "AssetManager.hpp"
#include <cassert>
#include <fstream>
#include <stdexcept>

std::vector<std::byte>
AssetManager::loadFileToBuffer(const std::filesystem::path& relativeFilePath) {
    const std::filesystem::path fullPath = basePath / relativeFilePath;
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        // Using .sring().c_str() instead of just .c_str() because just .c_str() returns a wchar_t
        // which is not supported by SDL_Log
        missingFileFatalError("Unable to open" + fullPath.string());
        return {};
    }
    const std::streamsize fileSize = file.tellg();
    if (fileSize <= 0) {
        missingFileFatalError("File is empty or invalid: " + fullPath.string());
        return {};
    }
    std::vector<std::byte> buffer(static_cast<size_t>(fileSize));
    file.seekg(0, std::ios::beg);
    // reinterpret_cast is simply to avoid compiler warnings, unlike with static_cast, the data is
    // not manipulated in any way
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        missingFileFatalError("Unable to read " + fullPath.string());
        return {};
    }
    SDL_Log("Got raw data from file %s", fullPath.filename().string().c_str());
    return buffer;
}

AssetManager::AssetManager(SDL_Renderer* renderer)
    : basePath{SDL_GetBasePath()}, renderer{renderer} {
    if (!renderer) {
        missingFileFatalError("SDL3 renderer is null");
        return;
    }
    textEngine = UniqueTextEngine(TTF_CreateRendererTextEngine(renderer));
    if (!textEngine) {
        std::string error = SDL_GetError();
        missingFileFatalError("Unable to create SDL3 text engine:\n" + error);
    }
}

TTF_TextEngine* AssetManager::getTextEngine() const {
    return textEngine.get();
}

TTF_Font* AssetManager::getFont(GameAssets::Fonts fontId, float ptSize, TTF_FontStyleFlags style) {
    assert(fontId < GameAssets::Fonts::FontsCount);
    if (ptSize <= 0) {
        ptSize = 12;
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Font size must be greater than zero. Defaulting to %.1f for \"%s\".",
            ptSize,
            GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)]
        );
    }
    ptSize *= TextResolutionScaleFactor;
    const float epsilon = 0.001f; // If difference is less than this value then its a match
    for (size_t i = 0; i < fontCache.size(); i++) {
        if (fontCache[i].fontId == fontId && std::abs(fontCache[i].ptSize - ptSize) < epsilon &&
            fontCache[i].style == style) {
            return fontCache[i].font.get();
        }
    }
    auto& rawData = fontData[static_cast<size_t>(fontId)];
    if (rawData.empty()) {
        std::filesystem::path relativeFilePath =
            GameAssets::Paths.Fonts / GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)];
        rawData = loadFileToBuffer(relativeFilePath);
    }
    SDL_IOStream* io = SDL_IOFromConstMem(rawData.data(), rawData.size());
    if (!io) {
        std::string message = "SDL_IOFromConstMem failed for font \"";
        message += GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)];
        message += "\":\n";
        message += SDL_GetError();
        missingFileFatalError(message);
        return nullptr;
    }
    TTF_Font* newFont = TTF_OpenFontIO(io, true, ptSize);
    if (!newFont) {
        std::string message = "TTF_OpenFontIO failed for font \"";
        message += GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)];
        message += "\":\n";
        message += SDL_GetError();
        missingFileFatalError(message);
        return nullptr;
    }
    TTF_SetFontStyle(newFont, style);
    CachedFont newCachedFont;
    newCachedFont.fontId = fontId;
    newCachedFont.font = UniqueFont(newFont);
    newCachedFont.ptSize = ptSize;
    newCachedFont.style = style;
    fontCache.push_back(
        std::move(newCachedFont)
    ); // std::move is needed since UniqueFonts are not copyable
    SDL_Log(
        "Loaded SDL3 ttf from file \"%s\"", GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)]
    );
    return newFont;
}

UniqueText AssetManager::getText(
    const std::string& text, GameAssets::Fonts fontId, float ptSize, TTF_FontStyleFlags style
) {
    if (!textEngine) {
        missingFileFatalError("Unable to get text. SDL3 text engine is null.");
        return nullptr;
    }
    TTF_Font* font = getFont(fontId, ptSize, style);
    if (!font) {
        std::string message = "Unable to get text for font \"";
        message += GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)];
        message += "\"";
        missingFileFatalError(message);
        return nullptr;
    }
    return UniqueText(TTF_CreateText(textEngine.get(), font, text.c_str(), text.length()));
}

void AssetManager::clearFontCache() {
    fontCache.clear();
    SDL_Log("Cleared font cache");
}

void AssetManager::clearFontData() {
    for (auto& data : fontData) {
        data.clear();
    }
    SDL_Log("Cleared raw font data");
}

std::string AssetManager::getFontPath(GameAssets::Fonts fontId) {
    assert(fontId < GameAssets::Fonts::FontsCount);
    std::filesystem::path path = basePath / GameAssets::Paths.Fonts /
                                 GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)];
    return path.string();
}

MIX_Audio* AssetManager::getSound(GameAssets::Sounds soundId, MIX_Mixer* mixerDevice) {
    assert(soundId < GameAssets::Sounds::SoundsCount);
    std::filesystem::path path = basePath / GameAssets::Paths.Sounds /
                                 GameAssets::FileNames.Sounds[static_cast<size_t>(soundId)];
    SDL_IOStream* io = SDL_IOFromFile(path.string().c_str(), "r");
    if (!io) {
        std::string message = "Unable to create SDL3 io for sound \"";
        message += GameAssets::FileNames.Sounds[static_cast<size_t>(soundId)];
        message += "\":\n";
        message += SDL_GetError();
        missingFileFatalError(message);
        return nullptr;
    }
    MIX_Audio* sound = MIX_LoadAudio_IO(mixerDevice, io, true, true);
    if (!sound) {
        std::string message = "Unable to load SDL3 io for sound \"";
        message += GameAssets::FileNames.Sounds[static_cast<size_t>(soundId)];
        message += "\":\n";
        message += SDL_GetError();
        missingFileFatalError(message);
        return nullptr;
    }
    SDL_Log(
        "Loaded sound from file \"%s\"", GameAssets::FileNames.Sounds[static_cast<size_t>(soundId)]
    );
    return sound;
}

MIX_Audio* AssetManager::getMusic(GameAssets::Music musicId, MIX_Mixer* mixerDevice) {
    assert(musicId < GameAssets::Music::MusicCount);
    std::filesystem::path path = basePath / GameAssets::Paths.Music /
                                 GameAssets::FileNames.Music[static_cast<size_t>(musicId)];
    SDL_IOStream* io = SDL_IOFromFile(path.string().c_str(), "r");
    if (!io) {
        std::string message = "Unable to create SDL3 io for music \"";
        message += GameAssets::FileNames.Music[static_cast<size_t>(musicId)];
        message += "\":\n";
        message += SDL_GetError();
        missingFileFatalError(message);
        return nullptr;
    }
    MIX_Audio* music = MIX_LoadAudio_IO(mixerDevice, io, false, true);
    if (!music) {
        std::string message = "Unable to load SDL3 io for music \"";
        message += GameAssets::FileNames.Music[static_cast<size_t>(musicId)];
        message += "\":\n";
        message += SDL_GetError();
        missingFileFatalError(message);
        return nullptr;
    }
    SDL_Log(
        "Loaded music from file \"%s\"", GameAssets::FileNames.Music[static_cast<size_t>(musicId)]
    );
    return music;
}

SDL_Texture* AssetManager::getTexture(GameAssets::Textures textureId) {
    assert(

        textureId < GameAssets::Textures::TexturesCount
    );
    if (!renderer) {
        std::string message = "Unable to load SDL3 texture from file \"";
        message += GameAssets::FileNames.Textures[static_cast<size_t>(textureId)];
        message += "\":\nSDl3 renderer is null";
        missingFileFatalError(message);
        return nullptr;
    }
    auto& texture = textureCache[static_cast<size_t>(textureId)];
    if (texture) {
        return texture.get();
    }
    std::filesystem::path relativePath =
        GameAssets::Paths.Textures / GameAssets::FileNames.Textures[static_cast<size_t>(textureId)];
    std::filesystem::path fullPath = basePath / relativePath;
    SDL_IOStream* io =
        SDL_IOFromFile(fullPath.string().c_str(), "r"); // "r" means open file for reading
    if (!io) {
        std::string message = "Unable to load SDL3 texture from file \"";
        message += GameAssets::FileNames.Textures[static_cast<size_t>(textureId)];
        message += "\":\n";
        message += SDL_GetError();
        missingFileFatalError(message);
        return nullptr;
    }
    texture.reset(IMG_LoadTexture_IO(renderer, io, true)); // Transfers ownership to the new pointer
    SDL_Log(
        "Loaded SDL3 texture from file \"%s\"",
        GameAssets::FileNames.Textures[static_cast<size_t>(textureId)]
    );
    return texture.get();
}

void AssetManager::missingFileFatalError(const std::string& message) {
    const char* title = "Missing File Error";
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", title, message.c_str());
    throw std::runtime_error(message);
}