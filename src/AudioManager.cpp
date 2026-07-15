#include "AudioManager.hpp"
#include <format>

AudioManager::AudioManager(AssetManager& assetManagerRef) : assetManager{&assetManagerRef} {
    if (!assetManager) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO,
            "Unable to create Audio Manager instance due to null asset manager ptr"
        );
        return;
    }
    mixerDevice = UniqueMixer(MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr));
    if (!mixerDevice) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO, "Unable to create SDL3 mixer device: %s", SDL_GetError()
        );
        return;
    }
    SDL_Log("Created SDL3 mixer device");
    for (size_t i = 0; i < SoundTrackCount; i++) {
        MIX_Track* rawTrack = MIX_CreateTrack(mixerDevice.get());
        if (!rawTrack) {
            SDL_LogError(
                SDL_LOG_CATEGORY_AUDIO,
                "Failed to create sound track index %zu: %s",
                i,
                SDL_GetError()
            );
            continue;
        }
        soundTracks[i] = UniqueTrack(rawTrack);
        MIX_TagTrack(soundTracks[i].get(), TagNames[static_cast<size_t>(AudioCategory::Sounds)]);
    }
    musicTrack = UniqueTrack(MIX_CreateTrack(mixerDevice.get()));
    if (musicTrack) {
        MIX_TagTrack(musicTrack.get(), TagNames[static_cast<size_t>(AudioCategory::Music)]);
    }
    SDL_Log("Created SDL3 mixer audio tracks");
    tagGain.fill(1.f);
}

bool AudioManager::playSound(std::string_view relativePath, unsigned int volume, float pitch) {
    if (!assetManager) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO,
            "Unable to play sound \"%.*s\" due to null SDL3 mixer music track",
            static_cast<int>(relativePath.length()),
            relativePath.data()
        );
        return false;
    }
    MIX_Audio* sound = assetManager->getAudio(relativePath, mixerDevice.get(), true);
    if (!sound) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_AUDIO,
            "Ignoring attempt to play null sound \"%.*s\"",
            static_cast<int>(relativePath.length()),
            relativePath.data()
        );
        return false;
    }
    MIX_Track* freeTrack = nullptr;
    for (size_t i = 0; i < SoundTrackCount; i++) {
        if (!MIX_TrackPlaying(soundTracks[i].get())) {
            freeTrack = soundTracks[i].get();
            break;
        }
    }
    if (!freeTrack) {
        freeTrack = soundTracks[0].get();
    }
    MIX_SetTrackFrequencyRatio(freeTrack, pitch);
    MIX_SetTrackGain(
        freeTrack,
        static_cast<float>(volume) / 100.f * tagGain[static_cast<size_t>(AudioCategory::Sounds)]
    );
    MIX_SetTrackAudio(freeTrack, sound);
    MIX_PlayTrack(freeTrack, 0);
    return true;
}

bool AudioManager::playMusic(
    std::string_view relativePath, unsigned int volume, float pitch, bool loop
) {
    if (!musicTrack) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO,
            "Unable to play music \"%.*s\" due to null SDL3 mixer music track",
            static_cast<int>(relativePath.length()),
            relativePath.data()
        );
        clearCurrentMusic();
        return false;
    }
    clearCurrentMusic();
    if (!assetManager) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO,
            "Unable to play music \"%.*s\" due to null asset manager ptr",
            static_cast<int>(relativePath.length()),
            relativePath.data()
        );
        clearCurrentMusic();
        return false;
    }
    currentMusic = assetManager->getAudio(relativePath, mixerDevice.get(), false);
    if (!currentMusic) {
        clearCurrentMusic();
        return false;
    }
    MIX_SetTrackFrequencyRatio(musicTrack.get(), pitch);
    float volumeFloat = static_cast<float>(volume);
    MIX_SetTrackGain(
        musicTrack.get(), volumeFloat / 100.f * tagGain[static_cast<size_t>(AudioCategory::Music)]
    );
    currentMusicVolume = volumeFloat / 100.f;
    MIX_SetTrackAudio(musicTrack.get(), currentMusic);
    SDL_PropertiesID properties = SDL_CreateProperties();
    if (loop) {
        // -1 loops infinitely, any positive integer would loop that amount of times.
        SDL_SetNumberProperty(properties, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    }
    MIX_PlayTrack(musicTrack.get(), properties);
    SDL_DestroyProperties(properties);
    currentMusicRelativePath = std::filesystem::path(relativePath);
    currentMusicName = currentMusicRelativePath.filename().string();
    return true;
}

unsigned int AudioManager::getVolume(AudioCategory category) {
    if (category >= AudioCategory::AudioCategoryCount) {
        return 0;
    }
    return static_cast<unsigned int>(SDL_roundf(tagGain[static_cast<size_t>(category)] * 100));
}

void AudioManager::setVolume(AudioCategory category, unsigned int volume) {
    if (category >= AudioCategory::AudioCategoryCount) {
        return;
    }
    float volumeFloat = static_cast<float>(volume) / 100.f;
    tagGain[static_cast<size_t>(category)] = volumeFloat;
    if (category == AudioCategory::Master) {
        MIX_SetMixerGain(mixerDevice.get(), volumeFloat);
        return;
    } else if (category == AudioCategory::Music) {
        MIX_SetTagGain(
            mixerDevice.get(),
            TagNames[static_cast<size_t>(AudioCategory::Music)],
            volumeFloat * currentMusicVolume
        );
    }
}

void AudioManager::pauseCategory(AudioCategory category) {
    if (category >= AudioCategory::AudioCategoryCount) {
        return;
    }
    if (category == AudioCategory::Master) {
        MIX_PauseAllTracks(mixerDevice.get());
        return;
    }
    MIX_PauseTag(mixerDevice.get(), TagNames[static_cast<size_t>(category)]);
}

void AudioManager::unpauseCategory(AudioCategory category) {
    if (category >= AudioCategory::AudioCategoryCount) {
        return;
    }
    if (category == AudioCategory::Master) {
        MIX_ResumeAllTracks(mixerDevice.get());
        return;
    }
    MIX_ResumeTag(mixerDevice.get(), TagNames[static_cast<size_t>(category)]);
}

void AudioManager::clearCurrentMusic() {
    if (!musicTrack) {
        return;
    }
    MIX_PauseTrack(musicTrack.get());
    MIX_SetTrackAudio(musicTrack.get(), nullptr);
    if (assetManager) {
        assetManager->unloadAudio(currentMusicRelativePath.generic_string());
    }
    currentMusic = nullptr;
    currentMusicName = "";
    currentMusicRelativePath = "";
}

bool AudioManager::isMusicPlaying() const {
    if (!currentMusic || !musicTrack) {
        return false;
    }
    if (isMusicLooping()) {
        return true;
    }
    Sint64 playbackFrames = MIX_GetTrackPlaybackPosition(musicTrack.get());
    Sint64 playbackPositionMS = MIX_TrackFramesToMS(musicTrack.get(), playbackFrames);
    MIX_Audio* music = MIX_GetTrackAudio(musicTrack.get());
    Sint64 durationFrames = MIX_GetAudioDuration(music);
    Sint64 musicLengthMS = MIX_TrackFramesToMS(musicTrack.get(), durationFrames);
    return playbackPositionMS != musicLengthMS;
}

bool AudioManager::isMusicLooping() const {
    return MIX_GetTrackLoops(musicTrack.get()) == -1 ? true : false;
}

float AudioManager::getMusicPitch() const {
    return MIX_GetTrackFrequencyRatio(musicTrack.get());
}

void AudioManager::setMusicPitch(float pitch) {
    if (pitch < 0.01f) {
        pitch = 0.01f;
    }
    MIX_SetTrackFrequencyRatio(musicTrack.get(), pitch);
}

std::string AudioManager::getCurrentMusicName() const {
    return currentMusicName;
}

Sint64 AudioManager::getMusicPlaybackPosition() const {
    Sint64 sampleFrames = MIX_GetTrackPlaybackPosition(musicTrack.get());
    return MIX_TrackFramesToMS(musicTrack.get(), sampleFrames) / 1000;
}

Sint64 AudioManager::getMusicTimeRemaining() const {
    Sint64 sampleFrames = MIX_GetTrackRemaining(musicTrack.get());
    return MIX_TrackFramesToMS(musicTrack.get(), sampleFrames) / 1000;
}

Sint64 AudioManager::getMusicLength() const {
    MIX_Audio* music = MIX_GetTrackAudio(musicTrack.get());
    Sint64 sampleFrames = MIX_GetAudioDuration(music);
    return MIX_TrackFramesToMS(musicTrack.get(), sampleFrames) / 1000;
}

std::string AudioManager::formattedMusicTime() const {
    Sint64 lengthSeconds = getMusicLength();
    Sint64 lengthMinutes = lengthSeconds / 60;
    lengthSeconds %= 60;
    Sint64 posSeconds = getMusicPlaybackPosition();
    Sint64 posMinutes = posSeconds / 60;
    posSeconds %= 60;
    return std::format(
        "{:02}:{:02}/{:02}:{:02}", posMinutes, posSeconds, lengthMinutes, lengthSeconds
    );
}