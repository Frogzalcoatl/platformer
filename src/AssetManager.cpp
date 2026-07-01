#include "AssetManager.hpp"
#include <cassert>
#include <fstream>

std::vector<std::byte>
AssetManager::loadFileToBuffer(const std::filesystem::path relativeFilePath) {
    const std::filesystem::path fullPath = basePath / relativeFilePath;
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        // Using .sring().c_str() instead of just .c_str() because just .c_str() returns a wchar_t
        // which is not supported by SDL_Log
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to open %s", fullPath.string().c_str());
        return {};
    }
    const std::streamsize fileSize = file.tellg();
    std::vector<std::byte> buffer(static_cast<size_t>(fileSize));
    file.seekg(0, std::ios::beg);
    // reinterpret_cast is simply to avoid compiler warnings, unlike with static_cast, the data is
    // not manipulated in any way
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Unable to read file %s", fullPath.string().c_str()
        );
        return {};
    }
    SDL_Log("Read file %s", fullPath.string().c_str());
    return buffer;
}

AssetManager::AssetManager(SDL_Renderer* renderer)
    : basePath{SDL_GetBasePath()}, renderer{renderer} {
    if (!renderer) {
        return;
    }
    textEngine = TTF_CreateRendererTextEngine(renderer);
    if (!textEngine) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Unable to create SDL text engine: %s", SDL_GetError()
        );
    }
    textureCache.fill(nullptr);
}

AssetManager::~AssetManager() {
    closeAll();
}

void AssetManager::closeAll() {
    if (fontCache.size() > 0) {
        for (const CachedFont& chachedFont : fontCache) {
            TTF_CloseFont(chachedFont.font);
        }
        fontCache.clear();
        SDL_Log("Closed cached fonts");
    }
    for (auto& texture : textureCache) {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }
    if (textEngine) {
        TTF_DestroyRendererTextEngine(textEngine);
        textEngine = nullptr;
        SDL_Log("Destroyed SDL_ttf text engine");
    }
}

TTF_TextEngine* AssetManager::getTextEngine() const {
    return textEngine;
}

TTF_Font* AssetManager::getFont(GameAssets::Fonts fontId, float ptSize, TTF_FontStyleFlags style) {
    assert(fontId >= static_cast<GameAssets::Fonts>(0) && fontId < GameAssets::Fonts::FontCount);
    if (ptSize <= 0) {
        return nullptr;
    }
    ptSize *= textResolutionScaleFactor;
    for (size_t i = 0; i < fontCache.size(); i++) {
        if (fontCache[i].fontId == fontId && fontCache[i].ptSize == ptSize &&
            fontCache[i].style == style) {
            return fontCache[i].font;
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
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL_IOFromConstMem failed for font \"%s\": %s",
            GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)],
            SDL_GetError()
        );
        return nullptr;
    }
    TTF_Font* newFont = TTF_OpenFontIO(io, true, ptSize);
    if (!newFont) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "TTF_OpenFontIO failed for font \"%s\": %s",
            GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)],
            SDL_GetError()
        );
        return nullptr;
    }
    TTF_SetFontStyle(newFont, style);
    CachedFont newCachedFont;
    newCachedFont.fontId = fontId;
    newCachedFont.font = newFont;
    newCachedFont.ptSize = ptSize;
    newCachedFont.style = style;
    fontCache.push_back(newCachedFont);
    SDL_Log(
        "Loaded SDL ttf from file \"%s\"", GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)]
    );
    return newFont;
}

TTF_Text* AssetManager::getText(
    std::string text, GameAssets::Fonts fontId, float ptSize, TTF_FontStyleFlags style
) {
    if (!textEngine) {
        return nullptr;
    }
    TTF_Font* font = getFont(fontId, ptSize, style);
    if (!font) {
        return nullptr;
    }
    return TTF_CreateText(textEngine, font, text.c_str(), text.length());
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

std::vector<std::byte> AssetManager::getSoundData(GameAssets::Sounds soundId) {
    assert(
        soundId >= static_cast<GameAssets::Sounds>(0) && soundId < GameAssets::Sounds::SoundsCount
    );
    std::filesystem::path relativePath =
        GameAssets::Paths.Sounds / GameAssets::FileNames.Sounds[static_cast<size_t>(soundId)];
    auto data = loadFileToBuffer(relativePath);
    SDL_Log(
        "Got raw sound data from file \"%s\"",
        GameAssets::FileNames.Sounds[static_cast<size_t>(soundId)]
    );
    return data;
}

std::vector<std::byte> AssetManager::getMusicData(GameAssets::Music musicId) {
    assert(musicId >= static_cast<GameAssets::Music>(0) && musicId < GameAssets::Music::MusicCount);
    std::filesystem::path relativePath =
        GameAssets::Paths.Music / GameAssets::FileNames.Music[static_cast<size_t>(musicId)];
    auto data = loadFileToBuffer(relativePath);
    SDL_Log(
        "Got raw music data from file \"%s\"",
        GameAssets::FileNames.Music[static_cast<size_t>(musicId)]
    );
    return data;
}

SDL_Texture* AssetManager::getTexture(GameAssets::Textures textureId) {
    assert(
        textureId >= static_cast<GameAssets::Textures>(0) &&
        textureId < GameAssets::Textures::TexturesCount
    );
    if (!renderer) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to load SDL texture \"%s\": renderer is null",
            GameAssets::FileNames.Textures[static_cast<size_t>(textureId)]
        );
        return nullptr;
    }
    auto& texture = textureCache[static_cast<size_t>(textureId)];
    if (texture) {
        return texture;
    }
    std::filesystem::path relativePath =
        GameAssets::Paths.Textures / GameAssets::FileNames.Textures[static_cast<size_t>(textureId)];
    std::filesystem::path fullPath = basePath / relativePath;
    // r means open file for reading
    texture = IMG_LoadTexture_IO(renderer, SDL_IOFromFile(fullPath.string().c_str(), "r"), true);
    if (!texture) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to load SDL texture \"%s\": %s",
            GameAssets::FileNames.Textures[static_cast<size_t>(textureId)],
            SDL_GetError()
        );
        return nullptr;
    }
    SDL_Log(
        "Loaded SDL texture from file \"%s\"",
        GameAssets::FileNames.Textures[static_cast<size_t>(textureId)]
    );
    return texture;
}