#pragma once
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <box2d/box2d.h>
#include <filesystem>
#include <imgui.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <vfs/vfs.hpp>

// Got idea to use deleter structs from AI. Makes sense to me.
struct SdlTextureDeleter {
    // Overloading the function call operator by using "operator()"
    // Doing things this way prevents unique_ptr from carrying an extra ptr for the delete func
    void operator()(SDL_Texture* t) const {
        if (t) {
            SDL_DestroyTexture(t);
        }
    }
};

struct TtfFontDeleter {
    void operator()(TTF_Font* f) const {
        if (f) {
            TTF_CloseFont(f);
        }
    }
};

struct TtfTextEngineDeleter {
    void operator()(TTF_TextEngine* t) const {
        if (t) {
            TTF_DestroyRendererTextEngine(t);
        }
    }
};

struct TtfTextDeleter {
    void operator()(TTF_Text* t) const {
        if (t) {
            TTF_DestroyText(t);
        }
    }
};

struct MixAudioDeleter {
    void operator()(MIX_Audio* a) const {
        if (a) {
            MIX_DestroyAudio(a);
        }
    }
};

using UniqueTexture = std::unique_ptr<SDL_Texture, SdlTextureDeleter>;
using UniqueFont = std::unique_ptr<TTF_Font, TtfFontDeleter>;
using UniqueTextEngine = std::unique_ptr<TTF_TextEngine, TtfTextEngineDeleter>;
using UniqueText = std::unique_ptr<TTF_Text, TtfTextDeleter>;
using UniqueAudio = std::unique_ptr<MIX_Audio, MixAudioDeleter>;

struct CachedFont {
    std::filesystem::path font_path;
    float pt_size;
    TTF_FontStyleFlags style;
    UniqueFont font;
};

struct StringHash {
    using is_transparent = void;
    size_t operator()(std::string_view sv) const {
        return std::hash<std::string_view>{}(sv);
    }
};

using FontDataMap =
    std::unordered_map<std::string, std::vector<std::byte>, StringHash, std::equal_to<>>;
using FontCacheVector = std::vector<CachedFont>;
using AudioCacheMap = std::unordered_map<std::string, UniqueAudio, StringHash, std::equal_to<>>;
using TextureCacheMap = std::unordered_map<std::string, UniqueTexture, StringHash, std::equal_to<>>;

enum class AssetTypes : uint8_t {
    audio,
    font_sdl,
    texture
};

struct FontInfo {
    float pt_size;
    TTF_FontStyleFlags style = TTF_STYLE_NORMAL;
    bool operator==(const FontInfo& other) const = default;
};

class AssetManager {
  private:
    VirtualFileSystem vfs_;
    UniqueTextEngine text_engine_;
    SDL_Renderer* renderer_;
    FontDataMap font_data_;
    FontCacheVector font_cache_;
    AudioCacheMap audio_cache_non_predecoded_;
    AudioCacheMap audio_cache_predecoded_;
    TextureCacheMap texture_cache_;

    std::string error_message_sdl_text(std::string_view text);

  public:
    AssetManager(SDL_Renderer* renderer);
    ~AssetManager();

    // Disable copying to protect the stability of memory buffers
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    const float text_render_scale = 10.f;
    const float text_world_size_multiplier = 0.04f;

    TTF_TextEngine* text_engine() const;

    TTF_Font*
    load_sdl_font(std::string_view relative_path, float pt_size, TTF_FontStyleFlags style);

    UniqueText get_sdl_text(
        std::string_view text,
        std::string_view relative_font_path,
        float pt_size,
        TTF_FontStyleFlags style = TTF_STYLE_NORMAL
    );

    ImFont* load_imgui_font(std::string_view relative_path, float pt_size);

    // predecode should be set to false for longer audio files like music
    MIX_Audio* load_audio(std::string_view relative_path, MIX_Mixer* mixer_device, bool predecode);

    SDL_Texture* load_texture(std::string_view relative_path);

    int add_controller_mappings(std::string_view relative_path);

    bool unload_sdl_font(
        std::string_view relative_path, float pt_size, TTF_FontStyleFlags style = TTF_STYLE_NORMAL
    );

    bool unload_audio(std::string_view relative_path, bool predecoded);

    bool unload_texture(std::string_view relative_path);
};