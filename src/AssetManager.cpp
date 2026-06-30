#include "AssetManager.hpp"
#include <cassert>
#include <fstream>

static std::vector<std::byte> loadFileToBuffer(const std::filesystem::path& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to open %s", filePath.c_str());
        return {};
    }
    const std::streamsize fileSize = file.tellg();
    std::vector<std::byte> buffer(static_cast<size_t>(fileSize));
    file.seekg(0, std::ios::beg);
    // reinterpret_cast is simply to avoid compiler warnings, unlike with static_cast, the data is
    // not manipulated in any way
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        return {};
    }
    return buffer;
}

AssetManager::AssetManager() {
    const char* rawPath = SDL_GetBasePath();
    std::filesystem::path basePath{rawPath};
    std::filesystem::path fontsPath = basePath / "assets" / "fonts";
    std::filesystem::path monocraftPath = fontsPath / "Monocraft.ttf";
    fontData[static_cast<size_t>(GameAssets::Font::Monocraft)] = loadFileToBuffer(monocraftPath);
}

TTF_Font* AssetManager::getFont(GameAssets::Font fontId, float ptSize, TTF_FontStyleFlags style) {
    assert(fontId >= static_cast<GameAssets::Font>(0) && fontId < GameAssets::Font::FontCount);
    if (ptSize <= 0) {
        return nullptr;
    }
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
    TTF_TextEngine* textEngine,
    std::string text,
    GameAssets::Font fontId,
    float ptSize,
    TTF_FontStyleFlags style
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