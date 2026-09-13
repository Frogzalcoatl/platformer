#include "assets/asset_manager.h"
#include "assets/asset_paths.h"
#include <algorithm>
#include <cstring>

AssetManager::AssetManager(SDL_Renderer* renderer)
    : vfs_(
          std::filesystem::path(SDL_GetBasePath() ? SDL_GetBasePath() : ""),
          PackFileName,
          GameVersion,
          AssetsFolderName
      ),
      renderer_{renderer} {
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL3 renderer for AssetManager is null");
        return;
    }
    text_engine_ = UniqueTextEngine(TTF_CreateRendererTextEngine(renderer));
    if (!text_engine_) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Unable to create SDL3 text engine: %s", SDL_GetError()
        );
    }
}

AssetManager::~AssetManager() {
    for (const auto& [path, texture] : texture_cache_) {
        SDL_Log("Unloaded SDL3 texture from file \"%s\"", path.c_str());
    }
    for (const auto& [path, audio] : audio_cache_predecoded_) {
        SDL_Log("Destroyed predecoded MIX_Audio from file \"%s\"", path.c_str());
    }
    for (const auto& [path, audio] : audio_cache_non_predecoded_) {
        SDL_Log("Destroyed non-predecoded MIX_Audio from file \"%s\"", path.c_str());
    }
    for (const auto& cached_font : font_cache_) {
        SDL_Log("Unloaded SDL3 ttf from file \"%s\"", cached_font.font_path.string().c_str());
    }
}

TTF_Font* AssetManager::load_sdl_font(
    std::string_view relative_path, float pt_size, TTF_FontStyleFlags style
) {
    if (pt_size <= 0) {
        pt_size = 12;
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Font size must be greater than zero. Defaulting to %.1f for \"%.*s\".",
            pt_size,
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
    }
    pt_size *= text_render_scale;
    const float epsilon = 0.001f;
    for (size_t i = 0; i < font_cache_.size(); i++) {
        if (font_cache_[i].font_path == relative_path &&
            std::abs(font_cache_[i].pt_size - pt_size) < epsilon && font_cache_[i].style == style) {
            return font_cache_[i].font.get();
        }
    }
    std::vector<std::byte>* raw_data = nullptr;
    auto it = font_data_.find(relative_path);
    if (it == font_data_.end()) {
        auto [inserted_it, success] =
            font_data_.emplace(std::string{relative_path}, vfs_.read_file(relative_path));
        raw_data = &inserted_it->second;
    } else {
        raw_data = &it->second;
    }
    SDL_IOStream* io = SDL_IOFromConstMem(raw_data->data(), raw_data->size());
    if (!io) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL_IOFromConstMem failed for font \"%.*s\": %s",
            static_cast<int>(relative_path.length()),
            relative_path.data(),
            SDL_GetError()
        );
        return nullptr;
    }
    TTF_Font* new_font = TTF_OpenFontIO(io, true, pt_size);
    if (!new_font) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "TTF_OpenFontIO failed for font \"%.*s\": %s",
            static_cast<int>(relative_path.length()),
            relative_path.data(),
            SDL_GetError()
        );
        return nullptr;
    }
    TTF_SetFontStyle(new_font, style);
    CachedFont new_cached_font;
    new_cached_font.font_path = relative_path;
    new_cached_font.font = UniqueFont(new_font, TtfFontDeleter());
    new_cached_font.pt_size = pt_size;
    new_cached_font.style = style;
    font_cache_.push_back(std::move(new_cached_font));
    SDL_Log(
        "Loaded SDL3 ttf from file \"%.*s\"",
        static_cast<int>(relative_path.length()),
        relative_path.data()
    );
    return new_font;
}

TTF_TextEngine* AssetManager::text_engine() const {
    return text_engine_.get();
}

std::string AssetManager::error_message_sdl_text(std::string_view text) {
    std::string message = "Unable to get SDL3 TTF text \"";
    size_t max_error_length = 128;
    message.append(text, 0, SDL_min(text.length(), max_error_length));
    if (text.length() > max_error_length) {
        message += "...";
    }
    message += "\": ";
    return message;
}

UniqueText AssetManager::get_sdl_text(
    std::string_view text,
    std::string_view relative_font_path,
    float pt_size,
    TTF_FontStyleFlags style
) {
    if (!text_engine_) {
        std::string message = error_message_sdl_text(text);
        message += "SDL3 text engine is null";
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", message.c_str());
        return nullptr;
    }
    TTF_Font* font = load_sdl_font(relative_font_path, pt_size, style);
    if (!font) {
        std::string message = error_message_sdl_text(text);
        message += "font \"";
        message += relative_font_path;
        message += "\" is null";
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", message.c_str());
        return nullptr;
    }
    TTF_Text* ttf_text = TTF_CreateText(text_engine_.get(), font, text.data(), text.length());
    if (!ttf_text) {
        std::string message = error_message_sdl_text(text);
        message += "TTF_CreateText failed - ";
        message += SDL_GetError();
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", message.c_str());
        return nullptr;
    }
    return UniqueText(ttf_text, TtfTextDeleter{});
}

ImFont* AssetManager::load_imgui_font(std::string_view relative_path, float pt_size) {
    std::vector<std::byte> data = vfs_.read_file(relative_path);
    if (data.empty()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to get ImGui font from file \"%.*s\"",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        return nullptr;
    }
    void* imgui_owned_buffer = ImGui::MemAlloc(data.size());
    std::memcpy(imgui_owned_buffer, data.data(), data.size());
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font =
        io.Fonts->AddFontFromMemoryTTF(imgui_owned_buffer, static_cast<int>(data.size()), pt_size);
    if (!font) {
        ImGui::MemFree(imgui_owned_buffer);
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to add ImGui font from memory TTF \"%.*s\"",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        return nullptr;
    }
    return font;
}

MIX_Audio*
AssetManager::load_audio(std::string_view relative_path, MIX_Mixer* mixer_device, bool predecode) {
    AudioCacheMap& audio_cache = predecode ? audio_cache_predecoded_ : audio_cache_non_predecoded_;
    auto it = audio_cache.find(relative_path);
    if (it != audio_cache.end()) {
        return it->second.get();
    }
    std::vector<std::byte> sound_data = vfs_.read_file(relative_path);
    if (sound_data.empty()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to get MIX_Audio from file \"%.*s\"",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        return nullptr;
    }
    SDL_IOStream* io = SDL_IOFromConstMem(sound_data.data(), sound_data.size());
    if (!io) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to create SDL3 io for MIX_Audio \"%.*s\": %s",
            static_cast<int>(relative_path.length()),
            relative_path.data(),
            SDL_GetError()
        );
        return nullptr;
    }
    MIX_Audio* raw_sound = MIX_LoadAudio_IO(mixer_device, io, predecode, true);
    if (!raw_sound) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to load SDL3 io for MIX_Audio \"%.*s\": %s",
            static_cast<int>(relative_path.length()),
            relative_path.data(),
            SDL_GetError()
        );
        return nullptr;
    }
    UniqueAudio sound(raw_sound, MixAudioDeleter());
    auto [inserted_it, success] = audio_cache.emplace(relative_path, std::move(sound));
    SDL_Log(
        "Loaded %s MIX_Audio from file \"%.*s\"",
        predecode ? "predecoded" : "non-predecoded",
        static_cast<int>(relative_path.length()),
        relative_path.data()
    );
    return inserted_it->second.get();
}

SDL_Texture* AssetManager::load_texture(std::string_view relative_path) {
    auto it = texture_cache_.find(relative_path);
    if (it != texture_cache_.end()) {
        return it->second.get();
    }
    if (!renderer_) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to load SDL3 texture from file \"%.*s\": SDL3 renderer is null",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        if (relative_path != asset_paths::textures::missing) {
            return load_texture(asset_paths::textures::missing);
        } else {
            return nullptr;
        }
    }
    std::vector<std::byte> texture_data = vfs_.read_file(relative_path);
    if (texture_data.empty()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to get SDL3 texture from file \"%.*s\"",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        if (relative_path != asset_paths::textures::missing) {
            return load_texture(asset_paths::textures::missing);
        } else {
            return nullptr;
        }
    }
    SDL_IOStream* io = SDL_IOFromConstMem(texture_data.data(), texture_data.size());
    if (!io) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to load SDL3 texture from file \"%.*s\": %s",
            static_cast<int>(relative_path.length()),
            relative_path.data(),
            SDL_GetError()
        );
        if (relative_path != asset_paths::textures::missing) {
            return load_texture(asset_paths::textures::missing);
        } else {
            return nullptr;
        }
    }
    SDL_Texture* raw_texture = IMG_LoadTexture_IO(renderer_, io, true);
    UniqueTexture texture(raw_texture, SdlTextureDeleter());
    auto [inserted_it, success] =
        texture_cache_.emplace(std::string{relative_path}, std::move(texture));
    SDL_SetTextureScaleMode(inserted_it->second.get(), SDL_SCALEMODE_PIXELART);
    SDL_Log(
        "Loaded SDL3 texture from file \"%.*s\"",
        static_cast<int>(relative_path.length()),
        relative_path.data()
    );
    return inserted_it->second.get();
}

int AssetManager::add_controller_mappings(std::string_view relative_path) {
    std::vector<std::byte> controller_data = vfs_.read_file(relative_path);
    if (controller_data.empty()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to init game controller database from file \"%.*s\"",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        return 0;
    }
    SDL_IOStream* io = SDL_IOFromConstMem(controller_data.data(), controller_data.size());
    if (!io) {
        SDL_Log(
            "Unable to create SDL3 IO for file \"%.*s\": %s",
            static_cast<int>(relative_path.length()),
            relative_path.data(),
            SDL_GetError()
        );
        return 0;
    }
    int result = SDL_AddGamepadMappingsFromIO(io, true);
    SDL_Log(
        "Added %d gamepad %s from file \"%.*s\"",
        result,
        result == 1 ? "mapping" : "mappings",
        static_cast<int>(relative_path.length()),
        relative_path.data()
    );
    return result;
}

bool AssetManager::unload_sdl_font(
    std::string_view relative_path, float pt_size, TTF_FontStyleFlags style
) {
    pt_size *= text_render_scale;
    const float epsilon = 0.001f;
    size_t size_before = font_cache_.size();
    std::erase_if(font_cache_, [&](const CachedFont& cached_font) {
        return (
            cached_font.font_path == relative_path &&
            std::abs(cached_font.pt_size - pt_size) < epsilon && cached_font.style == style
        );
    });
    bool was_unloaded = font_cache_.size() < size_before;
    if (was_unloaded) {
        SDL_Log(
            "Unloaded SDL3 ttf from file \"%.*s\"",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        bool path_in_use =
            std::any_of(font_cache_.begin(), font_cache_.end(), [&](const CachedFont& cached) {
                return cached.font_path == relative_path;
            });
        if (!path_in_use) {
            auto it = font_data_.find(relative_path);
            if (it != font_data_.end()) {
                font_data_.erase(it);
            }
        }
    }
    return was_unloaded;
}

bool AssetManager::unload_audio(std::string_view relative_path, bool predecoded) {
    AudioCacheMap& cache_map = predecoded ? audio_cache_predecoded_ : audio_cache_non_predecoded_;
    auto it = cache_map.find(relative_path);
    if (it != cache_map.end()) {
        cache_map.erase(it);
        SDL_Log(
            "Destroyed %s MIX_Audio from file \"%.*s\"",
            predecoded ? "predecoded" : "non-predecoded",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        return true;
    }
    return false;
}

bool AssetManager::unload_texture(std::string_view relative_path) {
    auto it = texture_cache_.find(relative_path);
    if (it != texture_cache_.end()) {
        texture_cache_.erase(it);
        SDL_Log(
            "Unloaded SDL3 texture from file \"%.*s\"",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        return true;
    } else {
        return false;
    }
}