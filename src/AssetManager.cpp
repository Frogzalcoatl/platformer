#include "AssetManager.hpp"
#include "AssetPaths.hpp"
#include <cstring>
#include <fstream>

AssetManager::AssetManager(SDL_Renderer* renderer)
    : vfs(std::filesystem::path(SDL_GetBasePath() ? SDL_GetBasePath() : ""),
          PackFileName,
          GameVersion,
          AssetsFolderName),
      renderer{renderer} {
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL3 renderer for AssetManager is null");
        return;
    }
    textEngine = UniqueTextEngine(TTF_CreateRendererTextEngine(renderer));
    if (!textEngine) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Unable to create SDL3 text engine: %s", SDL_GetError()
        );
    }
}

AssetManager::~AssetManager() {
    // Log all cached assets before destruction
    for (const auto& [path, texture] : textureCache) {
        SDL_Log("Unloaded SDL3 texture from file \"%s\"", path.c_str());
    }
    for (const auto& [path, audio] : audioCachePredecoded) {
        SDL_Log("Destroyed predecoded MIX_Audio from file \"%s\"", path.c_str());
    }
    for (const auto& [path, audio] : audioCacheNonPredecoded) {
        SDL_Log("Destroyed non-predecoded MIX_Audio from file \"%s\"", path.c_str());
    }
    for (const auto& cachedFont : fontCache) {
        SDL_Log("Unloaded SDL3 ttf from file \"%s\"", cachedFont.fontPath.string().c_str());
    }
    // All this stuff automatically destructs after this
}

TTF_Font*
AssetManager::getSDLFont(std::string_view relativePath, float ptSize, TTF_FontStyleFlags style) {
    std::string pathStr(relativePath);
    if (ptSize <= 0) {
        ptSize = 12;
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Font size must be greater than zero. Defaulting to %.1f for \"%s\".",
            ptSize,
            pathStr.c_str()
        );
    }
    ptSize *= TextRenderScale;
    const float epsilon = 0.001f; // If difference is less than this value then its a match
    for (size_t i = 0; i < fontCache.size(); i++) {
        if (fontCache[i].fontPath == relativePath &&
            std::abs(fontCache[i].ptSize - ptSize) < epsilon && fontCache[i].style == style) {
            return fontCache[i].font.get();
        }
    }
    auto& rawData = fontData[pathStr];
    if (rawData.empty()) {
        rawData = vfs.readFile(pathStr);
    }
    SDL_IOStream* io = SDL_IOFromConstMem(rawData.data(), rawData.size());
    if (!io) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL_IOFromConstMem failed for font \"%s\": %s",
            pathStr.c_str(),
            SDL_GetError()
        );
        return nullptr;
    }
    TTF_Font* newFont = TTF_OpenFontIO(io, true, ptSize);
    if (!newFont) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "TTF_OpenFontIO failed for font \"%s\": %s",
            pathStr.c_str(),
            SDL_GetError()
        );
        return nullptr;
    }
    TTF_SetFontStyle(newFont, style);
    CachedFont newCachedFont;
    newCachedFont.fontPath = relativePath;
    newCachedFont.font = UniqueFont(newFont, TTF_Font_Deleter());
    newCachedFont.ptSize = ptSize;
    newCachedFont.style = style;
    fontCache.push_back(
        std::move(newCachedFont)
    ); // std::move is needed since UniqueFonts are not copyable
    SDL_Log("Loaded SDL3 ttf from file \"%s\"", pathStr.c_str());
    return newFont;
}

TTF_TextEngine* AssetManager::getTextEngine() const {
    return textEngine.get();
}

UniqueText AssetManager::getSDLText(
    std::string_view text, std::string_view relativePath, float ptSize, TTF_FontStyleFlags style
) {
    if (!textEngine) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to get text. SDL3 text engine is null.");
        return nullptr;
    }
    TTF_Font* font = getSDLFont(relativePath, ptSize, style);
    if (!font) {
        std::string pathStr(relativePath);
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to get SDL3 TTF text \"%.*s\" using font \"%s\"",
            static_cast<int>(text.length()),
            text.data(),
            pathStr.c_str()
        );
        return nullptr;
    }
    TTF_Text* ttfText = TTF_CreateText(textEngine.get(), font, text.data(), text.length());
    if (!ttfText) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to create SDL3 TTF text \"%.*s\": %s",
            static_cast<int>(text.length()),
            text.data(),
            SDL_GetError()
        );
        return nullptr;
    }
    return UniqueText(ttfText, TTF_Text_Deleter{});
}

ImFont* AssetManager::getImGuiFont(std::string_view relativePath, float ptSize) {
    std::string pathStr(relativePath);
    std::vector<std::byte> data = vfs.readFile(pathStr);
    if (data.empty()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to get ImGui font from file \"%s\"",
            pathStr.c_str()
        );
        return nullptr;
    }
    void* imguiOwnedBuffer = ImGui::MemAlloc(data.size());
    std::memcpy(imguiOwnedBuffer, data.data(), data.size());
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font =
        io.Fonts->AddFontFromMemoryTTF(imguiOwnedBuffer, static_cast<int>(data.size()), ptSize);
    if (!font) {
        ImGui::MemFree(imguiOwnedBuffer);
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to add ImGui font from memory TTF \"%s\"",
            pathStr.c_str()
        );
        return nullptr;
    }
    return font;
}

MIX_Audio*
AssetManager::getAudio(std::string_view relativePath, MIX_Mixer* mixerDevice, bool predecode) {
    AudioCacheMap& audioCache = predecode ? audioCachePredecoded : audioCacheNonPredecoded;
    auto iterator = audioCache.find(relativePath);
    if (iterator != audioCache.end()) {
        return iterator->second.get();
    }
    std::string pathStr(relativePath);
    std::vector<std::byte> soundData = vfs.readFile(pathStr);
    if (soundData.empty()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to get MIX_Audio from file \"%s\"",
            pathStr.c_str()
        );
        return nullptr;
    }
    SDL_IOStream* io = SDL_IOFromConstMem(soundData.data(), soundData.size());
    if (!io) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to create SDL3 io for MIX_Audio \"%s\": %s",
            pathStr.c_str(),
            SDL_GetError()
        );
        return nullptr;
    }
    MIX_Audio* rawSound = MIX_LoadAudio_IO(mixerDevice, io, predecode, true);
    if (!rawSound) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to load SDL3 io for MIX_Audio \"%s\": %s",
            pathStr.c_str(),
            SDL_GetError()
        );
        return nullptr;
    }
    UniqueAudio sound(rawSound, MIX_Audio_Deleter());
    auto [insertedIterator, success] = audioCache.emplace(pathStr, std::move(sound));
    SDL_Log("Loaded audio from file \"%s\"", pathStr.c_str());
    return insertedIterator->second.get();
}

SDL_Texture* AssetManager::getTexture(std::string_view relativePath) {
    auto iterator = textureCache.find(relativePath);
    if (iterator != textureCache.end()) {
        return iterator->second.get();
    }
    std::string pathStr(relativePath);
    if (!renderer) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to load SDL3 texture from file \"%s\": SDL3 renderer is null",
            pathStr.c_str()
        );
        if (relativePath != AssetPaths::Textures::Missing) {
            return getTexture(AssetPaths::Textures::Missing);
        } else {
            return nullptr;
        }
    }
    std::vector<std::byte> textureData = vfs.readFile(pathStr);
    if (textureData.empty()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to get SDL3 texture from file \"%s\"",
            pathStr.c_str()
        );
        if (relativePath != AssetPaths::Textures::Missing) {
            return getTexture(AssetPaths::Textures::Missing);
        } else {
            return nullptr;
        }
    }
    SDL_IOStream* io = SDL_IOFromConstMem(textureData.data(), textureData.size());
    if (!io) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to load SDL3 texture from file \"%s\": %s",
            pathStr.c_str(),
            SDL_GetError()
        );
        if (relativePath != AssetPaths::Textures::Missing) {
            return getTexture(AssetPaths::Textures::Missing);
        } else {
            return nullptr;
        }
    }
    SDL_Texture* rawTexture = IMG_LoadTexture_IO(renderer, io, true);
    UniqueTexture texture(rawTexture, SDL_Texture_Deleter());
    auto [insertedIterator, success] = textureCache.emplace(pathStr, std::move(texture));
    SDL_SetTextureScaleMode(insertedIterator->second.get(), SDL_SCALEMODE_PIXELART);
    SDL_Log("Loaded SDL3 texture from file \"%s\"", pathStr.c_str());
    return insertedIterator->second.get();
}

int AssetManager::addGameControllerMappings(std::string_view relativePath) {
    std::string pathStr(relativePath);
    std::vector<std::byte> controllerData = vfs.readFile(pathStr);
    if (controllerData.empty()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to init game controller database from file \"%s\"",
            pathStr.c_str()
        );
        return 0;
    }
    SDL_IOStream* io = SDL_IOFromConstMem(controllerData.data(), controllerData.size());
    if (!io) {
        SDL_Log("Unable to create SDL3 IO for file \"%s\": %s", pathStr.c_str(), SDL_GetError());
        return 0;
    }
    int result = SDL_AddGamepadMappingsFromIO(io, true);
    SDL_Log("Added %d gampad mapping(s) from file \"%s\"", result, pathStr.c_str());
    return result;
}

bool AssetManager::unloadSDLFont(
    std::string_view relativePath, float ptSize, TTF_FontStyleFlags style
) {
    std::string pathStr(relativePath);
    ptSize *= TextRenderScale;
    const float epsilon = 0.001f;
    size_t sizeBefore = fontCache.size();
    std::erase_if(fontCache, [&](const CachedFont& cachedFont) {
        return (
            cachedFont.fontPath == relativePath && std::abs(cachedFont.ptSize - ptSize) < epsilon &&
            cachedFont.style == style
        );
    });
    bool wasUnloaded = fontCache.size() < sizeBefore;
    if (wasUnloaded) {
        SDL_Log("Unloaded SDL3 ttf from file \"%s\"", pathStr.c_str());
        bool pathStillInUse =
            std::any_of(fontCache.begin(), fontCache.end(), [&](const CachedFont& cached) {
                return cached.fontPath == relativePath;
            });
        if (!pathStillInUse) {
            fontData.erase(pathStr);
        }
    }
    return wasUnloaded;
}

bool AssetManager::unloadAudio(std::string_view relativePath) {
    std::string pathStr(relativePath);
    bool wasUnloaded = false;
    auto iteratorNon = audioCacheNonPredecoded.find(pathStr);
    if (iteratorNon != audioCacheNonPredecoded.end()) {
        audioCacheNonPredecoded.erase(iteratorNon);
        SDL_Log("Destroyed non-predecoded MIX_Audio from file \"%s\"", pathStr.c_str());
        wasUnloaded = true;
    }
    auto iteratorPre = audioCachePredecoded.find(pathStr);
    if (iteratorPre != audioCachePredecoded.end()) {
        audioCachePredecoded.erase(iteratorPre);
        SDL_Log("Destroyed predecoded MIX_Audio from file \"%s\"", pathStr.c_str());
        wasUnloaded = true;
    }
    return wasUnloaded;
}

bool AssetManager::unloadTexture(std::string_view relativePath) {
    std::string pathStr(relativePath);
    auto iterator = textureCache.find(pathStr);
    if (iterator != textureCache.end()) {
        textureCache.erase(iterator);
        SDL_Log("Unloaded SDL3 texture from file \"%s\"", pathStr.c_str());
        return true;
    } else {
        return false;
    }
}