#pragma once
#include "assets/asset_manager.h"
#include "platformer/game_events.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <array>
#include <filesystem>
#include <memory>
#include <string>

inline constexpr size_t sound_track_count = 32;

struct MixMixerDeleter {
    void operator()(MIX_Mixer* m) const {
        if (m) {
            MIX_DestroyMixer(m);
            SDL_Log("Destroyed SDL3 mixer device");
        }
    }
};

struct MixTrackDeleter {
    void operator()(MIX_Track* t) const {
        if (t) {
            MIX_DestroyTrack(t);
        }
    }
};

using UniqueMixer = std::unique_ptr<MIX_Mixer, MixMixerDeleter>;
using UniqueTrack = std::unique_ptr<MIX_Track, MixTrackDeleter>;

class AudioManager {
  private:
    AssetManager* asset_manager_;
    UniqueMixer mixer_device_;
    std::array<const char*, static_cast<size_t>(AudioCategory::audio_category_count)> tag_names_ = {
        "Master", "Sounds", "Music"
    };
    std::array<float, static_cast<size_t>(AudioCategory::audio_category_count)> tag_gain_ = {};
    std::array<UniqueTrack, sound_track_count> sound_tracks_ = {};
    UniqueTrack music_track_;
    float current_music_volume_ =
        1.f; // Separate volume multiplier, based on volume passed into playMusic func
    MIX_Audio* current_music_ = nullptr;
    std::filesystem::path current_music_relative_path_;
    std::string current_music_name_ = "";

  public:
    AudioManager(AssetManager& asset_manager);

    bool play_sound(std::string_view relative_path, unsigned int volume = 100, float pitch = 1.f);

    bool play_music(
        std::string_view relative_path,
        unsigned int volume = 100,
        float pitch = 1.f,
        bool loop = false
    );

    MIX_Mixer* get_mixer_device() const {
        return mixer_device_.get();
    }

    unsigned int get_volume(AudioCategory category);

    void set_volume(AudioCategory category, unsigned int volume);

    void pause_category(AudioCategory category);

    void unpause_category(AudioCategory category);

    void clear_current_music();

    bool is_music_playing() const;

    bool is_music_looping() const;

    float get_music_pitch() const;

    void set_music_pitch(float pitch);

    std::string get_current_music_name() const;

    Sint64 get_music_playback_position() const; // In seconds

    Sint64 get_music_time_remaining() const; // In seconds

    Sint64 get_music_length() const; // In seconds

    std::string formatted_music_time() const; // MM:SS
};