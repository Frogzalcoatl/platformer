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
#include <unordered_map>
#include <variant>
#include <vector>

// Got idea to use deleter structs from AI. Makes sense to me.
struct SDL_Texture_Deleter {
    // Overloading the function call operator by using "operator()"
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
struct MIX_Audio_Deleter {
    void operator()(MIX_Audio* a) const {
        if (a) {
            MIX_DestroyAudio(a);
        }
    }
};
using UniqueTexture = std::unique_ptr<SDL_Texture, SDL_Texture_Deleter>;
using UniqueFont = std::unique_ptr<TTF_Font, TTF_Font_Deleter>;
using UniqueTextEngine = std::unique_ptr<TTF_TextEngine, TTF_TextEngine_Deleter>;
using UniqueText = std::unique_ptr<TTF_Text, TTF_Text_Deleter>;
using UniqueAudio = std::unique_ptr<MIX_Audio, MIX_Audio_Deleter>;

struct CachedFont {
    std::filesystem::path fontPath;
    float ptSize;
    TTF_FontStyleFlags style;
    UniqueFont font;
};

// (Suggestion from ai)
// Prevents C++ from allocating an extra std::string when performing a lookup
struct StringHash {
    using is_transparent = void; // Enables heterogeneous lookup
    size_t operator()(std::string_view sv) const {
        return std::hash<std::string_view>{}(sv);
    }
};

using FontDataMap = std::unordered_map<
    std::string,            // KeyType
    std::vector<std::byte>, // ValueType
    StringHash,             // HashFunction
    std::equal_to<>         // KeyEqualityFunction
    >;
using FontCacheVector = std::vector<CachedFont>;
using AudioCacheMap = std::unordered_map<std::string, UniqueAudio, StringHash, std::equal_to<>>;
using TextureCacheMap = std::unordered_map<std::string, UniqueTexture, StringHash, std::equal_to<>>;

class AssetManager {
  private:
    VirtualFileSystem vfs;
    UniqueTextEngine textEngine;
    SDL_Renderer* renderer;
    FontDataMap fontData;
    FontCacheVector fontCache;
    AudioCacheMap audioCacheNonPredecoded;
    AudioCacheMap audioCachePredecoded;
    TextureCacheMap textureCache;

  public:
    AssetManager(SDL_Renderer* renderer);

    ~AssetManager();

    // Disable copying to protect the stability of memory buffers
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Render text at (pointSize * TextRenderScale) then scale down with multiplier so that it still
    // looks good while zoomed in
    const float TextRenderScale = 10.f;
    const float TextWorldSizeMultiplier = 0.04f;

    TTF_TextEngine* getTextEngine() const;

    // All relativePath args should use forward slashes
    TTF_Font* getSDLFont(std::string_view relativePath, float ptSize, TTF_FontStyleFlags style);

    UniqueText getSDLText(
        std::string_view text,
        std::string_view relativeFontPath,
        float ptSize,
        TTF_FontStyleFlags style = TTF_STYLE_NORMAL
    );

    ImFont* getImGuiFont(std::string_view relativePath, float ptSize);

    // predecode should be set to false for longer audio files like music
    MIX_Audio* getAudio(std::string_view relativePath, MIX_Mixer* mixerDevice, bool predecode);

    SDL_Texture* getTexture(std::string_view relativePath);

    int addGameControllerMappings(
        std::string_view relativePath
    ); // Returns number of controller mappings added

    // unloaders return true if asset was unloaded
    bool unloadSDLFont(
        std::string_view relativePath, float ptSize, TTF_FontStyleFlags style = TTF_STYLE_NORMAL
    );

    bool unloadAudio(std::string_view relativePath);

    bool unloadTexture(std::string_view relativePath);
};