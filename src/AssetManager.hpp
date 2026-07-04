#pragma once
#include "VirtualFileSystem.hpp"
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <array>
#include <box2d/box2d.h>
#include <filesystem>
#include <imgui.h>
#include <memory>
#include <variant>
#include <vector>

namespace GameAssets {
enum class Fonts : uint8_t {
    Monocraft,
    FontsCount
};
enum class Sounds : uint8_t {
    Fart,
    Jump,
    SoundsCount
};
enum class Music : uint8_t {
    M2023_4,
    M2023_11,
    M2023_14,
    M2023_23,
    M2023_29,
    M2023_35,
    M2023_37,
    M2024_3,
    M2024_5,
    M2024_7,
    M2024_8,
    M2026_2,
    M2026_3,
    M2026_4,
    M2026_5,
    M2026_6,
    MusicCount
};
enum class Textures : uint8_t {
    Test,
    TexturesCount
};
inline constexpr struct {
    const std::array<const char*, static_cast<size_t>(GameAssets::Fonts::FontsCount)> Fonts = {
        "monocraft.ttf"
    };
    // AudioManager loads all sounds at startup but only loads music when requested.
    // mp3 files not enabled
    const std::array<const char*, static_cast<size_t>(GameAssets::Sounds::SoundsCount)> Sounds = {
        "fart.wav", "jump.wav"
    };
    const std::array<const char*, static_cast<size_t>(GameAssets::Music::MusicCount)> Music = {
        "2023_4 (unfinished).ogg",
        "2023_11(3).ogg",
        "2023_14.ogg",
        "2023_23.ogg",
        "2023_29.ogg",
        "2023_35.ogg",
        "2023_37.ogg",
        "2024_3.ogg",
        "2024_5.ogg",
        "2024_7.ogg",
        "2024_8.ogg",
        "2026_2.ogg",
        "2026_3.ogg",
        "2026_4.ogg",
        "2026_5.ogg",
        "2026_6.ogg"
    };
    const std::array<const char*, static_cast<size_t>(GameAssets::Textures::TexturesCount)>
        Textures = {"test.png"};
} FileNames;
inline struct {
    std::filesystem::path Fonts = "fonts";
    std::filesystem::path Sounds = "sounds";
    std::filesystem::path Music = "music";
    std::filesystem::path Textures = "textures";
    std::filesystem::path Gamepads = "gamepads";
} Paths;
} // namespace GameAssets

struct SDL_Texture_Deleter {
    // Overloading the functrion call operator by using "operator()"
    // Doing things this way prevents unique_ptr from carrying an extra ptr for the delete func
    void operator()(SDL_Texture* t) const {
        if (t) {
            SDL_DestroyTexture(t);
        }
    }
};
struct TTF_Font_Deleter {
    void operator()(TTF_Font* f) const {
        if (f) {
            TTF_CloseFont(f);
        }
    }
};
struct TTF_TextEngine_Deleter {
    void operator()(TTF_TextEngine* t) const {
        if (t) {
            TTF_DestroyRendererTextEngine(t);
            SDL_Log("Destroyed SDL3_ttf text engine");
        }
    }
};
struct TTF_Text_Deleter {
    void operator()(TTF_Text* t) const {
        if (t) {
            TTF_DestroyText(t);
        }
    }
};
using UniqueTexture = std::unique_ptr<SDL_Texture, SDL_Texture_Deleter>;
using UniqueFont = std::unique_ptr<TTF_Font, TTF_Font_Deleter>;
using UniqueTextEngine = std::unique_ptr<TTF_TextEngine, TTF_TextEngine_Deleter>;
using UniqueText = std::unique_ptr<TTF_Text, TTF_Text_Deleter>;

struct CachedFont {
    GameAssets::Fonts fontId;
    float ptSize;
    TTF_FontStyleFlags style;
    UniqueFont font;
};

class AssetManager {
  private:
    VirtualFileSystem vfs;

    std::array<std::vector<std::byte>, static_cast<size_t>(GameAssets::Fonts::FontsCount)> fontData;
    std::vector<CachedFont> fontCache;
    std::array<UniqueTexture, static_cast<size_t>(GameAssets::Textures::TexturesCount)>
        textureCache;
    UniqueTextEngine textEngine;
    SDL_Renderer* renderer;

    TTF_Font* getFont(GameAssets::Fonts font, float ptSize, TTF_FontStyleFlags style);
    int addGameControllerMappings(
        const std::string& fileName
    ); // Returns number of controller mappings added

  public:
    AssetManager(SDL_Renderer* renderer);

    // Disable copying to protect the stability of memory buffers
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Render text at a high resolution then scale down so that it still looks good while zoomed in
    const float TextResolutionScaleFactor = 50.f;
    TTF_TextEngine* getTextEngine() const;
    UniqueText getText(
        const std::string& text,
        GameAssets::Fonts fontId,
        float ptSize,
        TTF_FontStyleFlags style = TTF_STYLE_NORMAL
    );
    ImFont* getImGuiFont(GameAssets::Fonts fontId, float ptSize);

    MIX_Audio* getSound(GameAssets::Sounds soundId, MIX_Mixer* mixerDevice);
    MIX_Audio* getMusic(GameAssets::Music musicId, MIX_Mixer* mixerDevice);

    SDL_Texture* getTexture(GameAssets::Textures textureId);

    int initSDLGameControllerDB(); // Returns number of controller mappings added
};