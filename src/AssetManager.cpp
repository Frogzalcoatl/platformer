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

TTF_Font* AssetManager::getFont(GameAssets::Fonts fontId, float ptSize, TTF_FontStyleFlags style) {
    assert(fontId < GameAssets::Fonts::FontsCount);
    const std::string fileName = GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)];
    if (ptSize <= 0) {
        ptSize = 12;
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Font size must be greater than zero. Defaulting to %.1f for \"%s\".",
            ptSize,
            fileName.c_str()
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
        std::filesystem::path relativeFilePath = GameAssets::Paths.Fonts / fileName;
        rawData = vfs.readFile(relativeFilePath.generic_string());
    }
    SDL_IOStream* io = SDL_IOFromConstMem(rawData.data(), rawData.size());
    if (!io) {
        std::string error = "SDL_IOFromConstMem failed for font \"";
        error += fileName;
        error += "\":\n";
        error += SDL_GetError();
        throw std::runtime_error(error);
        return nullptr;
    }
    TTF_Font* newFont = TTF_OpenFontIO(io, true, ptSize);
    if (!newFont) {
        std::string error = "TTF_OpenFontIO failed for font \"";
        error += fileName;
        error += "\":\n";
        error += SDL_GetError();
        throw std::runtime_error(error);
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
    SDL_Log("Loaded SDL3 ttf from file \"%s\"", fileName.c_str());
    return newFont;
}

UniqueText AssetManager::getText(
    const std::string& text, GameAssets::Fonts fontId, float ptSize, TTF_FontStyleFlags style
) {
    if (!textEngine) {
        throw std::runtime_error("Unable to get text. SDL3 text engine is null.");
    }
    TTF_Font* font = getFont(fontId, ptSize, style);
    if (!font) {
        std::string error = "Unable to get text for font \"";
        error += GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)];
        error += "\"";
        throw std::runtime_error(error);
    }
    return UniqueText(TTF_CreateText(textEngine.get(), font, text.c_str(), text.length()));
}

ImFont* AssetManager::getImGuiFont(GameAssets::Fonts fontId, float ptSize) {
    assert(fontId < GameAssets::Fonts::FontsCount);
    const std::string fileName = GameAssets::FileNames.Fonts[static_cast<size_t>(fontId)];
    std::filesystem::path relativePath = GameAssets::Paths.Fonts / fileName;
    std::vector<std::byte> fontData = vfs.readFile(relativePath.generic_string());
    if (fontData.empty()) {
        throw std::runtime_error("Unable to get ImGui Font from file \"" + fileName + "\"");
    }
    void* imguiOwnedBuffer = ImGui::MemAlloc(fontData.size());
    std::memcpy(imguiOwnedBuffer, fontData.data(), fontData.size());
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font =
        io.Fonts->AddFontFromMemoryTTF(imguiOwnedBuffer, static_cast<int>(fontData.size()), ptSize);
    if (!font) {
        ImGui::MemFree(imguiOwnedBuffer);
        throw std::runtime_error("Unable to add ImGui font from memory TTF \"" + fileName + "\"");
    }
    return font;
}

MIX_Audio* AssetManager::getSound(GameAssets::Sounds soundId, MIX_Mixer* mixerDevice) {
    assert(soundId < GameAssets::Sounds::SoundsCount);
    const std::string fileName = GameAssets::FileNames.Sounds[static_cast<size_t>(soundId)];
    std::filesystem::path relativePath = GameAssets::Paths.Sounds / fileName;
    std::vector<std::byte> soundData = vfs.readFile(relativePath.generic_string());
    if (soundData.empty()) {
        throw std::runtime_error("Unable to get MIX_Audio from file \"" + fileName + "\"");
    }
    SDL_IOStream* io = SDL_IOFromConstMem(soundData.data(), soundData.size());
    if (!io) {
        std::string error = "Unable to create SDL3 io for sound \"";
        error += fileName;
        error += "\":\n";
        error += SDL_GetError();
        throw std::runtime_error(error);
    }
    MIX_Audio* sound = MIX_LoadAudio_IO(mixerDevice, io, true, true);
    if (!sound) {
        std::string error = "Unable to load SDL3 io for sound \"";
        error += fileName;
        error += "\":\n";
        error += SDL_GetError();
        throw std::runtime_error(error);
    }
    SDL_Log("Loaded sound from file \"%s\"", fileName.c_str());
    return sound;
}

MIX_Audio* AssetManager::getMusic(GameAssets::Music musicId, MIX_Mixer* mixerDevice) {
    assert(musicId < GameAssets::Music::MusicCount);
    const std::string fileName = GameAssets::FileNames.Music[static_cast<size_t>(musicId)];
    std::filesystem::path relativePath = GameAssets::Paths.Music / fileName;
    std::vector<std::byte> musicData = vfs.readFile(relativePath.generic_string());
    if (musicData.empty()) {
        throw std::runtime_error("Unable to get MIX_Audio from file \"" + fileName + "\"");
    }
    SDL_IOStream* io = SDL_IOFromConstMem(musicData.data(), musicData.size());
    if (!io) {
        std::string error = "Unable to create SDL3 io for music \"";
        error += GameAssets::FileNames.Music[static_cast<size_t>(musicId)];
        error += "\":\n";
        error += SDL_GetError();
        throw std::runtime_error(error);
        return nullptr;
    }
    MIX_Audio* music = MIX_LoadAudio_IO(mixerDevice, io, false, true);
    if (!music) {
        std::string error = "Unable to load SDL3 io for music \"";
        error += GameAssets::FileNames.Music[static_cast<size_t>(musicId)];
        error += "\":\n";
        error += SDL_GetError();
        throw std::runtime_error(error);
        return nullptr;
    }
    SDL_Log(
        "Loaded music from file \"%s\"", GameAssets::FileNames.Music[static_cast<size_t>(musicId)]
    );
    return music;
}

SDL_Texture* AssetManager::getTexture(GameAssets::Textures textureId) {
    assert(textureId < GameAssets::Textures::TexturesCount);
    if (textureId == GameAssets::Textures::None) {
        return nullptr;
    }
    const std::string fileName = GameAssets::FileNames.Textures[static_cast<size_t>(textureId)];
    if (!renderer) {
        std::string error = "Unable to load SDL3 texture from file \"";
        error += fileName;
        error += "\":\nSDl3 renderer is null";
        throw std::runtime_error(error);
        return nullptr;
    }
    auto& texture = textureCache[static_cast<size_t>(textureId)];
    if (texture) {
        return texture.get();
    }
    std::filesystem::path relativePath = GameAssets::Paths.Textures / fileName;
    std::vector<std::byte> textureData = vfs.readFile(relativePath.generic_string());
    if (textureData.empty()) {
        throw std::runtime_error("Unable to get SDL3 texture from file \"" + fileName + "\"");
    }
    SDL_IOStream* io = SDL_IOFromConstMem(textureData.data(), textureData.size());
    if (!io) {
        std::string error = "Unable to load SDL3 texture from file \"";
        error += GameAssets::FileNames.Textures[static_cast<size_t>(textureId)];
        error += "\":\n";
        error += SDL_GetError();
        throw std::runtime_error(error);
        return nullptr;
    }
    texture.reset(IMG_LoadTexture_IO(renderer, io, true)); // Transfers ownership to the new pointer
    SDL_Log(
        "Loaded SDL3 texture from file \"%s\"",
        GameAssets::FileNames.Textures[static_cast<size_t>(textureId)]
    );
    SDL_SetTextureScaleMode(texture.get(), SDL_SCALEMODE_PIXELART);
    return texture.get();
}

int AssetManager::addGameControllerMappings(const std::string& fileName) {
    std::filesystem::path path = GameAssets::Paths.Gamepads / fileName;
    std::vector<std::byte> controllerData = vfs.readFile(path.generic_string());
    if (controllerData.empty()) {
        std::string error = "Unable to init game controller database from file \"";
        error += fileName;
        error += "\"";
        throw std::runtime_error(error);
    }
    SDL_IOStream* io = SDL_IOFromConstMem(controllerData.data(), controllerData.size());
    if (!io) {
        std::string error = "Unable to create SDL3 IO for file \"";
        error += fileName;
        error += "\"";
        throw std::runtime_error(error);
    }
    int result = SDL_AddGamepadMappingsFromIO(io, true);
    SDL_Log("Added %d gampad mapping(s) from file \"%s\"", result, fileName.c_str());
    return result;
}

int AssetManager::initSDLGameControllerDB() {
    int result = addGameControllerMappings("gamecontrollerdb.txt");
    result += addGameControllerMappings("personalcontrollerdb.txt");
    return result;
}