#include "system/audio_manager.h"
#include <format>

AudioManager::AudioManager(AssetManager& asset_manager_ref) : asset_manager_{&asset_manager_ref} {
    if (!asset_manager_) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO,
            "Unable to create Audio Manager instance due to null asset manager ptr"
        );
        return;
    }
    mixer_device_ = UniqueMixer(MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr));
    if (!mixer_device_) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO, "Unable to create SDL3 mixer device: %s", SDL_GetError()
        );
        return;
    }
    SDL_Log("Created SDL3 mixer device");
    for (size_t i = 0; i < sound_track_count; i++) {
        MIX_Track* raw_track = MIX_CreateTrack(mixer_device_.get());
        if (!raw_track) {
            SDL_LogError(
                SDL_LOG_CATEGORY_AUDIO,
                "Failed to create sound track index %zu: %s",
                i,
                SDL_GetError()
            );
            continue;
        }
        sound_tracks_[i] = UniqueTrack(raw_track);
        MIX_TagTrack(
            sound_tracks_[i].get(), tag_names_[static_cast<size_t>(AudioCategory::sounds)]
        );
    }
    music_track_ = UniqueTrack(MIX_CreateTrack(mixer_device_.get()));
    if (music_track_) {
        MIX_TagTrack(music_track_.get(), tag_names_[static_cast<size_t>(AudioCategory::music)]);
    }
    SDL_Log("Created SDL3 mixer audio tracks");
    tag_gain_.fill(1.f);
}

bool AudioManager::play_sound(std::string_view relative_path, unsigned int volume, float pitch) {
    if (!asset_manager_) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO,
            "Unable to play sound \"%.*s\" due to null SDL3 mixer music track",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        return false;
    }
    MIX_Audio* sound = asset_manager_->load_audio(relative_path, mixer_device_.get(), true);
    if (!sound) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_AUDIO,
            "Ignoring attempt to play null sound \"%.*s\"",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        return false;
    }
    MIX_Track* free_track = nullptr;
    for (size_t i = 0; i < sound_track_count; i++) {
        if (!MIX_TrackPlaying(sound_tracks_[i].get())) {
            free_track = sound_tracks_[i].get();
            break;
        }
    }
    if (!free_track) {
        free_track = sound_tracks_[0].get();
    }
    MIX_SetTrackFrequencyRatio(free_track, pitch);
    MIX_SetTrackGain(
        free_track,
        static_cast<float>(volume) / 100.f * tag_gain_[static_cast<size_t>(AudioCategory::sounds)]
    );
    MIX_SetTrackAudio(free_track, sound);
    MIX_PlayTrack(free_track, 0);
    return true;
}

bool AudioManager::play_music(
    std::string_view relative_path, unsigned int volume, float pitch, bool loop
) {
    if (!music_track_) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO,
            "Unable to play music \"%.*s\" due to null SDL3 mixer music track",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        clear_current_music();
        return false;
    }
    clear_current_music();
    if (!asset_manager_) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO,
            "Unable to play music \"%.*s\" due to null asset manager ptr",
            static_cast<int>(relative_path.length()),
            relative_path.data()
        );
        clear_current_music();
        return false;
    }
    current_music_ = asset_manager_->load_audio(relative_path, mixer_device_.get(), false);
    if (!current_music_) {
        clear_current_music();
        return false;
    }
    MIX_SetTrackFrequencyRatio(music_track_.get(), pitch);
    float volume_float = static_cast<float>(volume);
    MIX_SetTrackGain(
        music_track_.get(),
        volume_float / 100.f * tag_gain_[static_cast<size_t>(AudioCategory::music)]
    );
    current_music_volume_ = volume_float / 100.f;
    MIX_SetTrackAudio(music_track_.get(), current_music_);
    SDL_PropertiesID properties = SDL_CreateProperties();
    if (loop) {
        // -1 loops infinitely, any positive integer would loop that amount of times.
        SDL_SetNumberProperty(properties, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    }
    MIX_PlayTrack(music_track_.get(), properties);
    SDL_DestroyProperties(properties);
    current_music_relative_path_ = std::filesystem::path(relative_path);
    current_music_name_ = current_music_relative_path_.filename().string();
    return true;
}

unsigned int AudioManager::get_volume(AudioCategory category) {
    if (category >= AudioCategory::audio_category_count) {
        return 0;
    }
    return static_cast<unsigned int>(SDL_roundf(tag_gain_[static_cast<size_t>(category)] * 100));
}

void AudioManager::set_volume(AudioCategory category, unsigned int volume) {
    if (category >= AudioCategory::audio_category_count) {
        return;
    }
    float volume_float = static_cast<float>(volume) / 100.f;
    tag_gain_[static_cast<size_t>(category)] = volume_float;
    if (category == AudioCategory::master) {
        MIX_SetMixerGain(mixer_device_.get(), volume_float);
        return;
    } else if (category == AudioCategory::music) {
        MIX_SetTagGain(
            mixer_device_.get(),
            tag_names_[static_cast<size_t>(AudioCategory::music)],
            volume_float * current_music_volume_
        );
    }
}

void AudioManager::pause_category(AudioCategory category) {
    if (category >= AudioCategory::audio_category_count) {
        return;
    }
    if (category == AudioCategory::master) {
        MIX_PauseAllTracks(mixer_device_.get());
        return;
    }
    MIX_PauseTag(mixer_device_.get(), tag_names_[static_cast<size_t>(category)]);
}

void AudioManager::unpause_category(AudioCategory category) {
    if (category >= AudioCategory::audio_category_count) {
        return;
    }
    if (category == AudioCategory::master) {
        MIX_ResumeAllTracks(mixer_device_.get());
        return;
    }
    MIX_ResumeTag(mixer_device_.get(), tag_names_[static_cast<size_t>(category)]);
}

void AudioManager::clear_current_music() {
    if (!music_track_) {
        return;
    }
    MIX_PauseTrack(music_track_.get());
    MIX_SetTrackAudio(music_track_.get(), nullptr);
    if (asset_manager_) {
        asset_manager_->unload_audio(current_music_relative_path_.generic_string(), false);
    }
    current_music_ = nullptr;
    current_music_name_ = "";
    current_music_relative_path_ = "";
}

bool AudioManager::is_music_playing() const {
    if (!current_music_ || !music_track_) {
        return false;
    }
    if (is_music_looping()) {
        return true;
    }
    Sint64 playback_frames = MIX_GetTrackPlaybackPosition(music_track_.get());
    Sint64 playback_position_ms = MIX_TrackFramesToMS(music_track_.get(), playback_frames);
    MIX_Audio* music = MIX_GetTrackAudio(music_track_.get());
    Sint64 duration_frames = MIX_GetAudioDuration(music);
    Sint64 music_length_ms = MIX_TrackFramesToMS(music_track_.get(), duration_frames);
    return playback_position_ms != music_length_ms;
}

bool AudioManager::is_music_looping() const {
    return MIX_GetTrackLoops(music_track_.get()) == -1;
}

float AudioManager::get_music_pitch() const {
    return MIX_GetTrackFrequencyRatio(music_track_.get());
}

void AudioManager::set_music_pitch(float pitch) {
    if (pitch < 0.01f) {
        pitch = 0.01f;
    }
    MIX_SetTrackFrequencyRatio(music_track_.get(), pitch);
}

std::string AudioManager::get_current_music_name() const {
    return current_music_name_;
}

Sint64 AudioManager::get_music_playback_position() const {
    Sint64 sample_frames = MIX_GetTrackPlaybackPosition(music_track_.get());
    return MIX_TrackFramesToMS(music_track_.get(), sample_frames) / 1000;
}

Sint64 AudioManager::get_music_time_remaining() const {
    Sint64 sample_frames = MIX_GetTrackRemaining(music_track_.get());
    return MIX_TrackFramesToMS(music_track_.get(), sample_frames) / 1000;
}

Sint64 AudioManager::get_music_length() const {
    MIX_Audio* music = MIX_GetTrackAudio(music_track_.get());
    Sint64 sample_frames = MIX_GetAudioDuration(music);
    return MIX_TrackFramesToMS(music_track_.get(), sample_frames) / 1000;
}

std::string AudioManager::formatted_music_time() const {
    Sint64 length_seconds = get_music_length();
    Sint64 length_minutes = length_seconds / 60;
    length_seconds %= 60;
    Sint64 pos_seconds = get_music_playback_position();
    Sint64 pos_minutes = pos_seconds / 60;
    pos_seconds %= 60;
    return std::format(
        "{:02}:{:02}/{:02}:{:02}", pos_minutes, pos_seconds, length_minutes, length_seconds
    );
}