#include "AssetManager.hpp"
#include <cassert>
#include <fstream>
#include <stdexcept>

AssetManager::AssetManager(SDL_Renderer* renderer)
    : vfs{SDL_GetBasePath(), DatFileName, GameVersion, AssetsFolderName}, renderer{renderer} {
    if (!renderer) {
        throw std::runtime_error("SDL3 renderer for AssetManager is null");
        return;
    }
    textEngine = UniqueTextEngine(TTF_CreateRendererTextEngine(renderer));
    if (!textEngine) {
        std::string error = SDL_GetError();
        throw std::runtime_error("Unable to create SDL3 text engine:\n" + error);
    }
}

TTF_TextEngine* AssetManager::getTextEngine() const {
    return textEngine.get();
}

TTF_Font*
AssetManager::getSDLFont(std::string_view relativePathStr, float ptSize, TTF_FontStyleFlags style) {
    std::filesystem::path relativePath = relativePathStr;
    std::string pathStr = relativePath.generic_string();
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
        std::string error = "SDL_IOFromConstMem failed for font \"";
        error += pathStr;
        error += "\":\n";
        error += SDL_GetError();
        throw std::runtime_error(error);
        return nullptr;
    }
    TTF_Font* newFont = TTF_OpenFontIO(io, true, ptSize);
    if (!newFont) {
        std::string error = "TTF_OpenFontIO failed for font \"";
        error += pathStr;
        error += "\":\n";
        error += SDL_GetError();
        throw std::runtime_error(error);
        return nullptr;
    }
    TTF_SetFontStyle(newFont, style);
    CachedFont newCachedFont;
    newCachedFont.fontPath = relativePath;
    newCachedFont.font = UniqueFont(newFont, TTF_Font_Deleter(pathStr));
    newCachedFont.ptSize = ptSize;
    newCachedFont.style = style;
    fontCache.push_back(
        std::move(newCachedFont)
    ); // std::move is needed since UniqueFonts are not copyable
    SDL_Log("Loaded SDL3 ttf from file \"%s\"", pathStr.c_str());
    return newFont;
}

UniqueText AssetManager::getSDLText(
    std::string_view text, std::string_view relativePathStr, float ptSize, TTF_FontStyleFlags style
) {
    if (!textEngine) {
        throw std::runtime_error("Unable to get text. SDL3 text engine is null.");
    }
    TTF_Font* font = getSDLFont(relativePathStr, ptSize, style);
    if (!font) {
        std::string error = "Unable to get text \"";
        error += text;
        error += "\" using font \"";
        error += relativePathStr;
        error += "\"";
        throw std::runtime_error(error);
    }
    TTF_Text* ttfText = TTF_CreateText(textEngine.get(), font, text.data(), text.length());
    if (!ttfText) {
        std::string error = "Unable to get text \"";
        error += text;
        error += "\": ";
        error += SDL_GetError();
        throw std::runtime_error(error);
    }
    return UniqueText(ttfText, TTF_Text_Deleter{});
}

ImFont* AssetManager::getImGuiFont(std::string_view relativePathStr, float ptSize) {
    std::filesystem::path relativePath = relativePathStr;
    std::string pathStr = relativePath.generic_string();
    std::vector<std::byte> fontData = vfs.readFile(pathStr);
    if (fontData.empty()) {
        std::string error = "Unable to get ImGui Font from file \"";
        error += pathStr;
        error += "\"";
        throw std::runtime_error(error);
    }
    void* imguiOwnedBuffer = ImGui::MemAlloc(fontData.size());
    std::memcpy(imguiOwnedBuffer, fontData.data(), fontData.size());
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font =
        io.Fonts->AddFontFromMemoryTTF(imguiOwnedBuffer, static_cast<int>(fontData.size()), ptSize);
    if (!font) {
        ImGui::MemFree(imguiOwnedBuffer);
        std::string error = "Unable to add ImGui font from memory TTF \"";
        error += pathStr;
        error += "\"";
        throw std::runtime_error(error);
    }
    return font;
}

MIX_Audio*
AssetManager::getAudio(std::string_view relativePathStr, MIX_Mixer* mixerDevice, bool predecode) {
    AudioCacheMap& audioCache = predecode ? audioCachePredecoded : audioCacheNonPredecoded;
    auto iterator = audioCache.find(relativePathStr);
    if (iterator != audioCache.end()) {
        return iterator->second.get();
    }
    std::filesystem::path relativePath = relativePathStr;
    std::string pathStr = relativePath.generic_string();
    std::vector<std::byte> soundData = vfs.readFile(pathStr);
    if (soundData.empty()) {
        throw std::runtime_error("Unable to get MIX_Audio from file \"" + pathStr + "\"");
    }
    SDL_IOStream* io = SDL_IOFromConstMem(soundData.data(), soundData.size());
    if (!io) {
        std::string error = "Unable to create SDL3 io for MIX_Audio \"";
        error += pathStr;
        error += "\":\n";
        error += SDL_GetError();
        throw std::runtime_error(error);
    }
    MIX_Audio* rawSound = MIX_LoadAudio_IO(mixerDevice, io, predecode, true);
    if (!rawSound) {
        std::string error = "Unable to load SDL3 io for MIX_Audio \"";
        error += pathStr;
        error += "\":\n";
        error += SDL_GetError();
        throw std::runtime_error(error);
    }
    UniqueAudio sound(rawSound, MIX_Audio_Deleter(pathStr));
    auto [insertedIterator, success] = audioCache.emplace(pathStr, std::move(sound));
    SDL_Log("Loaded sound from file \"%s\"", pathStr.c_str());
    return insertedIterator->second.get();
}

SDL_Texture* AssetManager::getTexture(std::string_view relativePathStr) {
    auto iterator = textureCache.find(relativePathStr);
    if (iterator != textureCache.end()) {
        return iterator->second.get();
    }
    std::filesystem::path relativePath = relativePathStr;
    std::string pathStr = relativePath.generic_string();
    if (!renderer) {
        std::string error = "Unable to load SDL3 texture from file \"";
        error += pathStr;
        error += "\":\nSDl3 renderer is null";
        throw std::runtime_error(error);
        return nullptr;
    }
    std::vector<std::byte> textureData = vfs.readFile(pathStr);
    if (textureData.empty()) {
        std::string error = "Unable to get SDL3 texture from file \"";
        error += pathStr;
        error += "\"";
        throw std::runtime_error(error);
    }
    SDL_IOStream* io = SDL_IOFromConstMem(textureData.data(), textureData.size());
    if (!io) {
        std::string error = "Unable to load SDL3 texture from file \"";
        error += pathStr;
        error += "\":\n";
        error += SDL_GetError();
        throw std::runtime_error(error);
    }
    SDL_Texture* rawTexture = IMG_LoadTexture_IO(renderer, io, true);
    UniqueTexture texture(rawTexture, SDL_Texture_Deleter(pathStr));
    auto [insertedIterator, success] = textureCache.emplace(pathStr, std::move(texture));
    SDL_SetTextureScaleMode(insertedIterator->second.get(), SDL_SCALEMODE_PIXELART);
    SDL_Log("Loaded SDL3 texture from file \"%s\"", pathStr.c_str());
    return insertedIterator->second.get();
}

int AssetManager::addGameControllerMappings(std::string_view fileName) {
    std::filesystem::path relativePath(GamepadsFolderName);
    relativePath /= fileName;
    std::string pathStr = relativePath.generic_string();
    std::vector<std::byte> controllerData = vfs.readFile(pathStr);
    if (controllerData.empty()) {
        std::string error = "Unable to init game controller database from file \"";
        error += pathStr;
        error += "\"";
        throw std::runtime_error(error);
    }
    SDL_IOStream* io = SDL_IOFromConstMem(controllerData.data(), controllerData.size());
    if (!io) {
        std::string error = "Unable to create SDL3 IO for file \"";
        error += pathStr;
        error += "\"";
        throw std::runtime_error(error);
    }
    int result = SDL_AddGamepadMappingsFromIO(io, true);
    SDL_Log("Added %d gampad mapping(s) from file \"%s\"", result, pathStr.c_str());
    return result;
}

int AssetManager::initSDLGameControllerDB() {
    int result = addGameControllerMappings("gamecontrollerdb.txt");
    result += addGameControllerMappings("personalcontrollerdb.txt");
    return result;
}