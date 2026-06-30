#pragma once
#include <SDL3_ttf/SDL_ttf.h>
#include <array>
#include <box2d/box2d.h>
#include <filesystem>
#include <vector>

namespace GameAssets {
enum class Font : uint8_t { Monocraft, FontCount };
enum class Sound : uint8_t { Test, SoundCount };
} // namespace GameAssets

struct CachedFont {
    GameAssets::Font fontId;
    float ptSize;
    TTF_FontStyleFlags style;
    TTF_Font* font;
};

class AssetManager {
  private:
    SDL_Renderer* sdlRenderer;
    std::array<std::vector<std::byte>, static_cast<size_t>(GameAssets::Font::FontCount)> fontData =
        {};
    std::vector<CachedFont> fontCache;
    TTF_Font* getFont(GameAssets::Font font, float ptSize, TTF_FontStyleFlags style);

  public:
    AssetManager();

    // Disable copying to protect the stability of memory buffers
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    TTF_Text* getText(
        TTF_TextEngine* textEngine,
        std::string text,
        GameAssets::Font fontId,
        float ptSize,
        TTF_FontStyleFlags style = TTF_STYLE_NORMAL
    );
};