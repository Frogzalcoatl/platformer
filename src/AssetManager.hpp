#pragma once
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <array>
#include <box2d/box2d.h>
#include <filesystem>
#include <variant>
#include <vector>

// std::string_view is a efficient read-only version of std::string
inline constexpr std::string_view AssetsFolderName = "assets";
inline constexpr size_t GameAssetTypeCount = 2;

namespace GameAssets {
enum class Fonts : uint8_t {
    Monocraft,
    FontCount
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
    MusicCount
};
enum class Textures : uint8_t {
    Test,
    TexturesCount
};
inline constexpr struct {
    const std::array<const char*, static_cast<size_t>(GameAssets::Fonts::FontCount)> Fonts = {
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
        "2026_5.ogg"
    };
    const std::array<const char*, static_cast<size_t>(GameAssets::Textures::TexturesCount)>
        Textures = {"test.png"};
} FileNames;
inline struct {
    std::filesystem::path Fonts = std::filesystem::path(AssetsFolderName) / "fonts";
    std::filesystem::path Sounds = std::filesystem::path(AssetsFolderName) / "sounds";
    std::filesystem::path Music = std::filesystem::path(AssetsFolderName) / "music";
    std::filesystem::path Textures = std::filesystem::path(AssetsFolderName) / "textures";
} Paths;
} // namespace GameAssets

struct CachedFont {
    GameAssets::Fonts fontId;
    float ptSize;
    TTF_FontStyleFlags style;
    TTF_Font* font;
};

class AssetManager {
  private:
    const std::filesystem::path basePath;
    std::vector<std::byte> loadFileToBuffer(const std::filesystem::path relativeFilePath);
    std::array<std::vector<std::byte>, static_cast<size_t>(GameAssets::Fonts::FontCount)> fontData;
    std::vector<CachedFont> fontCache;
    std::array<SDL_Texture*, static_cast<size_t>(GameAssets::Textures::TexturesCount)> textureCache;
    TTF_Font* getFont(GameAssets::Fonts font, float ptSize, TTF_FontStyleFlags style);
    TTF_TextEngine* textEngine;
    SDL_Renderer* renderer;

  public:
    AssetManager(SDL_Renderer* renderer);
    ~AssetManager();

    void closeAll();

    // Disable copying to protect the stability of memory buffers
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Render text at a high resolution then scale down so that it still looks good while zoomed in
    float textResolutionScaleFactor = 50.f;
    TTF_TextEngine* getTextEngine() const;
    TTF_Text* getText(
        std::string text,
        GameAssets::Fonts fontId,
        float ptSize,
        TTF_FontStyleFlags style = TTF_STYLE_NORMAL
    );
    // Cache includes styling, data is the raw bytes that are referenced to generate styled fonts
    void clearFontCache();
    void clearFontData();
    std::string getFontPath(GameAssets::Fonts fontId);

    MIX_Audio* getSound(GameAssets::Sounds soundId, MIX_Mixer* mixerDevice);
    MIX_Audio* getMusic(GameAssets::Music musicId, MIX_Mixer* mixerDevice);

    SDL_Texture* getTexture(GameAssets::Textures textureId);
};