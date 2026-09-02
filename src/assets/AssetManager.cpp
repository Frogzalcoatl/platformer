#include "assets/AssetManager.hpp"
#include "assets/AssetPaths.hpp"
#include <algorithm>
#include <cstring>

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
    if (ptSize <= 0) {
        ptSize = 12;
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Font size must be greater than zero. Defaulting to %.1f for \"%.*s\".",
            ptSize,
            static_cast<int>(relativePath.length()),
            relativePath.data()
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
    std::vector<std::byte>* rawDataPtr = nullptr;
    auto it = fontData.find(relativePath);
    if (it == fontData.end()) {
        auto [insertedIt, success] =
            fontData.emplace(std::string{relativePath}, vfs.readFile(relativePath));
        rawDataPtr = &insertedIt->second;
    } else {
        rawDataPtr = &it->second;
    }
    SDL_IOStream* io = SDL_IOFromConstMem(rawDataPtr->data(), rawDataPtr->size());
    if (!io) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL_IOFromConstMem failed for font \"%.*s\": %s",
            static_cast<int>(relativePath.length()),
            relativePath.data(),
            SDL_GetError()
        );
        return nullptr;
    }
    TTF_Font* newFont = TTF_OpenFontIO(io, true, ptSize);
    if (!newFont) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "TTF_OpenFontIO failed for font \"%.*s\": %s",
            static_cast<int>(relativePath.length()),
            relativePath.data(),
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
    SDL_Log(
        "Loaded SDL3 ttf from file \"%.*s\"",
        static_cast<int>(relativePath.length()),
        relativePath.data()
    );
    return newFont;
}

TTF_TextEngine* AssetManager::getTextEngine() const {
    return textEngine.get();
}

std::string AssetManager::getSDLTextErrorMessage(std::string_view text) {
    std::string message = "Unable to get SDL3 TTF text \"";
    size_t MaxTextLength = 128;
    message.append(text, 0, SDL_min(text.length(), MaxTextLength));
    if (text.length() > MaxTextLength) {
        message += "...";
    }
    message += "\": ";
    return message;
}

UniqueText AssetManager::getSDLText(
    std::string_view text, std::string_view relativeFontPath, float ptSize, TTF_FontStyleFlags style
) {
    if (!textEngine) {
        std::string message = getSDLTextErrorMessage(text);
        message += "SDL3 text engine is null";
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", message.c_str());
        return nullptr;
    }
    TTF_Font* font = getSDLFont(relativeFontPath, ptSize, style);
    if (!font) {
        std::string message = getSDLTextErrorMessage(text);
        message += "font \"";
        message += relativeFontPath;
        message += "\" is null";
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", message.c_str());
        return nullptr;
    }
    TTF_Text* ttfText = TTF_CreateText(textEngine.get(), font, text.data(), text.length());
    if (!ttfText) {
        std::string message = getSDLTextErrorMessage(text);
        message += "TTF_CreateText failed - ";
        message += SDL_GetError();
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", message.c_str());
        return nullptr;
    }
    return UniqueText(ttfText, TTF_Text_Deleter{});
}

ImFont* AssetManager::getImGuiFont(std::string_view relativePath, float ptSize) {
    std::vector<std::byte> data = vfs.readFile(relativePath);
    if (data.empty()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to get ImGui font from file \"%.*s\"",
            static_cast<int>(relativePath.length()),
            relativePath.data()
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
            "Unable to add ImGui font from memory TTF \"%.*s\"",
            static_cast<int>(relativePath.length()),
            relativePath.data()
        );
        return nullptr;
    }
    return font;
}

MIX_Audio*
AssetManager::getAudio(std::string_view relativePath, MIX_Mixer* mixerDevice, bool predecode) {
    AudioCacheMap& audioCache = predecode ? audioCachePredecoded : audioCacheNonPredecoded;
    auto it = audioCache.find(relativePath);
    if (it != audioCache.end()) {
        return it->second.get();
    }
    std::vector<std::byte> soundData = vfs.readFile(relativePath);
    if (soundData.empty()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to get MIX_Audio from file \"%.*s\"",
            static_cast<int>(relativePath.length()),
            relativePath.data()
        );
        return nullptr;
    }
    SDL_IOStream* io = SDL_IOFromConstMem(soundData.data(), soundData.size());
    if (!io) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to create SDL3 io for MIX_Audio \"%.*s\": %s",
            static_cast<int>(relativePath.length()),
            relativePath.data(),
            SDL_GetError()
        );
        return nullptr;
    }
    MIX_Audio* rawSound = MIX_LoadAudio_IO(mixerDevice, io, predecode, true);
    if (!rawSound) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to load SDL3 io for MIX_Audio \"%.*s\": %s",
            static_cast<int>(relativePath.length()),
            relativePath.data(),
            SDL_GetError()
        );
        return nullptr;
    }
    UniqueAudio sound(rawSound, MIX_Audio_Deleter());
    auto [insertedIterator, success] = audioCache.emplace(relativePath, std::move(sound));
    SDL_Log(
        "Loaded %s MIX_Audio from file \"%.*s\"",
        predecode ? "predecoded" : "non-predecoded",
        static_cast<int>(relativePath.length()),
        relativePath.data()
    );
    return insertedIterator->second.get();
}

SDL_Texture* AssetManager::getTexture(std::string_view relativePath) {
    auto it = textureCache.find(relativePath);
    if (it != textureCache.end()) {
        return it->second.get();
    }
    if (!renderer) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to load SDL3 texture from file \"%.*s\": SDL3 renderer is null",
            static_cast<int>(relativePath.length()),
            relativePath.data()
        );
        if (relativePath != AssetPaths::Textures::Missing) {
            return getTexture(AssetPaths::Textures::Missing);
        } else {
            return nullptr;
        }
    }
    std::vector<std::byte> textureData = vfs.readFile(relativePath);
    if (textureData.empty()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to get SDL3 texture from file \"%.*s\"",
            static_cast<int>(relativePath.length()),
            relativePath.data()
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
            "Unable to load SDL3 texture from file \"%.*s\": %s",
            static_cast<int>(relativePath.length()),
            relativePath.data(),
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
    auto [insertedIterator, success] =
        textureCache.emplace(std::string{relativePath}, std::move(texture));
    SDL_SetTextureScaleMode(insertedIterator->second.get(), SDL_SCALEMODE_PIXELART);
    SDL_Log(
        "Loaded SDL3 texture from file \"%.*s\"",
        static_cast<int>(relativePath.length()),
        relativePath.data()
    );
    return insertedIterator->second.get();
}

int AssetManager::addGameControllerMappings(std::string_view relativePath) {
    std::vector<std::byte> controllerData = vfs.readFile(relativePath);
    if (controllerData.empty()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to init game controller database from file \"%.*s\"",
            static_cast<int>(relativePath.length()),
            relativePath.data()
        );
        return 0;
    }
    SDL_IOStream* io = SDL_IOFromConstMem(controllerData.data(), controllerData.size());
    if (!io) {
        SDL_Log(
            "Unable to create SDL3 IO for file \"%.*s\": %s",
            static_cast<int>(relativePath.length()),
            relativePath.data(),
            SDL_GetError()
        );
        return 0;
    }
    int result = SDL_AddGamepadMappingsFromIO(io, true);
    SDL_Log(
        "Added %d gamepad %s from file \"%.*s\"",
        result,
        result == 1 ? "mapping" : "mappings",
        static_cast<int>(relativePath.length()),
        relativePath.data()
    );
    return result;
}

bool AssetManager::unloadSDLFont(
    std::string_view relativePath, float ptSize, TTF_FontStyleFlags style
) {
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
        SDL_Log(
            "Unloaded SDL3 ttf from file \"%.*s\"",
            static_cast<int>(relativePath.length()),
            relativePath.data()
        );
        bool pathStillInUse =
            std::any_of(fontCache.begin(), fontCache.end(), [&](const CachedFont& cached) {
                return cached.fontPath == relativePath;
            });
        if (!pathStillInUse) {
            auto it = fontData.find(relativePath);
            if (it != fontData.end()) {
                fontData.erase(it);
            }
        }
    }
    return wasUnloaded;
}

bool AssetManager::unloadAudio(std::string_view relativePath, bool predecoded) {
    AudioCacheMap& cacheMap = predecoded ? audioCachePredecoded : audioCacheNonPredecoded;
    auto it = cacheMap.find(relativePath);
    if (it != cacheMap.end()) {
        cacheMap.erase(it);
        SDL_Log(
            "Destroyed %s MIX_Audio from file \"%.*s\"",
            predecoded ? "predecoded" : "non-predecoded",
            static_cast<int>(relativePath.length()),
            relativePath.data()
        );
        return true;
    }
    return false;
}

bool AssetManager::unloadTexture(std::string_view relativePath) {
    auto it = textureCache.find(relativePath);
    if (it != textureCache.end()) {
        textureCache.erase(it);
        SDL_Log(
            "Unloaded SDL3 texture from file \"%.*s\"",
            static_cast<int>(relativePath.length()),
            relativePath.data()
        );
        return true;
    } else {
        return false;
    }
}