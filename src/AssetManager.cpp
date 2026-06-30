#include "AssetManager.hpp"
#include <cassert>
#include <fstream>

static std::vector<std::byte> loadFileToBuffer(const std::filesystem::path& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        // Using .sring().c_str() instead of just .c_str() because just .c_str() returns a wchar_t
        // which is not supported by SDL_Log
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to open %s", filePath.string().c_str());
        return {};
    }
    const std::streamsize fileSize = file.tellg();
    std::vector<std::byte> buffer(static_cast<size_t>(fileSize));
    file.seekg(0, std::ios::beg);
    // reinterpret_cast is simply to avoid compiler warnings, unlike with static_cast, the data is
    // not manipulated in any way
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Unable to read file %s", filePath.string().c_str()
        );
        return {};
    }
    SDL_Log("Read file %s", filePath.string().c_str());
    return buffer;
}

AssetManager::AssetManager(SDL_Renderer* renderer) {
    const char* rawPath = SDL_GetBasePath();
    std::filesystem::path basePath{rawPath};
    std::filesystem::path fontsPath = basePath / "assets" / "fonts";
    std::filesystem::path monocraftPath = fontsPath / "Monocraft.ttf";
    fontData[static_cast<size_t>(GameAssets::Font::Monocraft)] = loadFileToBuffer(monocraftPath);
    textEngine = TTF_CreateRendererTextEngine(renderer);
    if (!textEngine) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Unable to create SDL text engine: %s", SDL_GetError()
        );
    }
}

void AssetManager::closeAll() {
    for (const CachedFont& chachedFont : fontCache) {
        TTF_CloseFont(chachedFont.font);
    }
    SDL_Log("Closed cached fonts");
    TTF_DestroyRendererTextEngine(textEngine);
    SDL_Log("Destroyed SDL_ttf text engine");
}

TTF_Font* AssetManager::getFont(GameAssets::Font fontId, float ptSize, TTF_FontStyleFlags style) {
    assert(fontId >= static_cast<GameAssets::Font>(0) && fontId < GameAssets::Font::FontCount);
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
    const auto& rawData = fontData[static_cast<size_t>(fontId)];
    SDL_IOStream* io = SDL_IOFromConstMem(rawData.data(), rawData.size());
    if (!io) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_IOFromConstMem failed: %d", fontId);
        return nullptr;
    }
    TTF_Font* newFont = TTF_OpenFontIO(io, true, ptSize);
    if (!newFont) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_OpenFontIO failed: %s", SDL_GetError());
        return nullptr;
    }
    TTF_SetFontStyle(newFont, style);
    CachedFont newCachedFont;
    newCachedFont.fontId = fontId;
    newCachedFont.font = newFont;
    newCachedFont.ptSize = ptSize;
    newCachedFont.style = style;
    fontCache.push_back(newCachedFont);
    return newFont;
}

TTF_Text* AssetManager::getText(
    std::string text, GameAssets::Font fontId, float ptSize, TTF_FontStyleFlags style
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

TTF_TextEngine* AssetManager::getTextEngine() const {
    return textEngine;
}