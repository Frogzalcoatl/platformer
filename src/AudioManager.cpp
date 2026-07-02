#include "AudioManager.hpp"
#include <cassert>

AudioManager::AudioManager(AssetManager& assetManager) : assets{&assetManager} {
    if (!assets) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO,
            "Unable to create Audio Manager instance due to null asset manager ptr"
        );
        return;
    }
    mixerDevice = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!mixerDevice) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO, "Unable to create SDL3 mixer device: %s", SDL_GetError()
        );
        return;
    }
    SDL_Log("Created SDL mixer device");
    for (size_t i = 0; i < SoundTrackCount; i++) {
        soundTracks[i] = MIX_CreateTrack(mixerDevice);
        MIX_TagTrack(soundTracks[i], SoundTag);
    }
    musicTrack = MIX_CreateTrack(mixerDevice);
    MIX_TagTrack(musicTrack, MusicTag);
    SDL_Log("Created SDL3 mixer audio tracks");
    loadedSounds.fill(nullptr);
    for (size_t i = 0; i < static_cast<size_t>(GameAssets::Sounds::SoundsCount); i++) {
        const auto rawData = assets->getSoundData(static_cast<GameAssets::Sounds>(i));
        if (rawData.empty()) {
            continue;
        }
        SDL_IOStream* io = SDL_IOFromConstMem(rawData.data(), rawData.size());
        if (!io) {
            SDL_LogWarn(
                SDL_LOG_CATEGORY_AUDIO,
                "Unable to create SDL io for sound \"%s\": %s",
                GameAssets::FileNames.Sounds[i],
                SDL_GetError()
            );
            continue;
        }
        loadedSounds[i] = MIX_LoadAudio_IO(mixerDevice, io, true, true);
        if (!loadedSounds[i]) {
            SDL_LogError(
                SDL_LOG_CATEGORY_AUDIO,
                "Unable to load audio io for sound \"%s\": %s",
                GameAssets::FileNames.Sounds[i],
                SDL_GetError()
            );
            continue;
        }
    }
    volumeMultipliers.fill(1.f);
}

AudioManager::~AudioManager() {
    if (musicTrack) {
        MIX_SetTrackAudio(musicTrack, nullptr);
        MIX_DestroyTrack(musicTrack);
        musicTrack = nullptr;
        SDL_Log("Destroyed SDL Mixer music track");
    }
    for (size_t i = 0; i < SoundTrackCount; i++) {
        if (soundTracks[i]) {
            MIX_SetTrackAudio(soundTracks[i], nullptr);
            MIX_DestroyTrack(soundTracks[i]);
            soundTracks[i] = nullptr;
            SDL_Log("Destroyed SDL Mixer sound track at index %zu", i);
        }
    }
    for (size_t i = 0; i < static_cast<size_t>(GameAssets::Sounds::SoundsCount); i++) {
        if (loadedSounds[i]) {
            MIX_DestroyAudio(loadedSounds[i]);
            loadedSounds[i] = nullptr;
        }
    }
    if (currentMusic) {
        MIX_DestroyAudio(currentMusic);
        currentMusic = nullptr;
    }
    if (mixerDevice) {
        MIX_DestroyMixer(mixerDevice);
        mixerDevice = nullptr;
        SDL_Log("Destroyed SDL mixer device");
    }
}

void AudioManager::playSound(GameAssets::Sounds soundId, unsigned int volume, float pitch) {
    assert(
        soundId >= static_cast<GameAssets::Sounds>(0) && soundId < GameAssets::Sounds::SoundsCount
    );
    MIX_Audio* sound = loadedSounds[static_cast<size_t>(soundId)];
    if (!sound) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_AUDIO,
            "Ignoring attempt to play null sound \"%s\"",
            GameAssets::FileNames.Sounds[static_cast<size_t>(soundId)]
        );
        return;
    }
    MIX_Track* freeTrack = nullptr;
    for (size_t i = 0; i < SoundTrackCount; i++) {
        if (!MIX_TrackPlaying(soundTracks[i])) {
            freeTrack = soundTracks[i];
            break;
        }
    }
    if (!freeTrack) {
        freeTrack = soundTracks[0];
    }
    MIX_SetTrackFrequencyRatio(freeTrack, pitch);
    MIX_SetTagGain(
        mixerDevice,
        SoundTag,
        static_cast<float>(volume) / 100.f *
            volumeMultipliers[static_cast<size_t>(AudioCategory::Sounds)]
    );
    MIX_SetTrackAudio(freeTrack, sound);
    MIX_PlayTrack(freeTrack, 0);
}

void AudioManager::playMusic(
    GameAssets::Music musicId, unsigned int volume, float pitch, bool loop
) {
    assert(musicId >= static_cast<GameAssets::Music>(0) && musicId < GameAssets::Music::MusicCount);
    MIX_PauseTrack(musicTrack);
    if (currentMusic) {
        MIX_DestroyAudio(currentMusic);
        currentMusic = nullptr;
    }
    if (!assets) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO,
            "Unable to play music %s due to null asset manager ptr",
            GameAssets::FileNames.Music[static_cast<size_t>(musicId)]
        );
        return;
    }
    const auto rawData = assets->getMusicData(musicId);
    if (rawData.empty()) {
        // Already logs error to console inside getMusicData func
        return;
    }
    SDL_IOStream* io = SDL_IOFromConstMem(rawData.data(), rawData.size());
    if (!io) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_AUDIO,
            "Unable to create SDL io for music \"%s\": %s",
            GameAssets::FileNames.Music[static_cast<size_t>(musicId)],
            SDL_GetError()
        );
        return;
    }
    currentMusic = MIX_LoadAudio_IO(mixerDevice, io, false, true);
    if (!currentMusic) {
        SDL_LogError(
            SDL_LOG_CATEGORY_AUDIO,
            "Mixer failed to load audio IO for music \"%s\": %s",
            GameAssets::FileNames.Music[static_cast<size_t>(musicId)],
            SDL_GetError()
        );
        return;
    }
    MIX_SetTrackFrequencyRatio(musicTrack, pitch);
    MIX_SetTagGain(
        mixerDevice,
        MusicTag,
        volume / 100.f * volumeMultipliers[static_cast<size_t>(AudioCategory::Music)]
    );
    currentMusicVolume = volume / 100.f;
    MIX_SetTrackAudio(musicTrack, currentMusic);
    SDL_PropertiesID properties = SDL_CreateProperties();
    if (loop) {
        // -1 loops infinitely, any positive integer would loop that amount of times.
        SDL_SetNumberProperty(properties, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    }
    MIX_PlayTrack(musicTrack, properties);
    SDL_DestroyProperties(properties);
    currentMusicName = GameAssets::FileNames.Music[static_cast<size_t>(musicId)];
}

void AudioManager::setVolume(AudioCategory category, unsigned int volume) {
    assert(
        category >= static_cast<AudioCategory>(0) && category < AudioCategory::AudioCategoryCount
    );
    float volumeFloat = volume / 100.f;
    volumeMultipliers[static_cast<size_t>(category)] = volumeFloat;
    if (category == AudioCategory::Master) {
        MIX_SetMixerGain(mixerDevice, volumeFloat);
    } else if (category == AudioCategory::Music) {
        MIX_SetTrackGain(musicTrack, volumeFloat * currentMusicVolume);
    }
}

void AudioManager::setMusicPitch(float pitch) {
    MIX_SetTrackFrequencyRatio(musicTrack, pitch);
}

unsigned int AudioManager::getVolume(AudioCategory category) {
    assert(
        category >= static_cast<AudioCategory>(0) && category < AudioCategory::AudioCategoryCount
    );
    return static_cast<unsigned int>(
        SDL_roundf(volumeMultipliers[static_cast<size_t>(category)] * 100)
    );
}

const char* AudioManager::getCurrentMusicName() const {
    return currentMusicName;
}

float AudioManager::getMusicPitch() const {
    return MIX_GetTrackFrequencyRatio(musicTrack);
}

bool AudioManager::isMusicLooping() const {
    return MIX_GetTrackLoops(musicTrack) == -1 ? true : false;
}

Sint64 AudioManager::getMusicPlaybackPosition() const {
    Sint64 sampleFrames = MIX_GetTrackPlaybackPosition(musicTrack);
    return MIX_TrackFramesToMS(musicTrack, sampleFrames) / 1000;
}

Sint64 AudioManager::getMusicTimeRemaining() const {
    Sint64 sampleFrames = MIX_GetTrackRemaining(musicTrack);
    return MIX_TrackFramesToMS(musicTrack, sampleFrames) / 1000;
}

Sint64 AudioManager::getMusicLength() const {
    // Using this instead of currentMusic in case currentMusic has stopped playing
    MIX_Audio* music = MIX_GetTrackAudio(musicTrack);
    Sint64 sampleFrames = MIX_GetAudioDuration(music);
    return MIX_TrackFramesToMS(musicTrack, sampleFrames) / 1000;
}

std::string AudioManager::formattedMusicTime() const {
    Sint64 lengthSeconds = getMusicLength();
    Sint64 lengthMinutes = lengthSeconds / 60;
    lengthSeconds %= 60;
    Sint64 posSeconds = getMusicPlaybackPosition();
    Sint64 posMinutes = lengthSeconds / 60;
    posSeconds %= 60;
    char buf[16];
    std::snprintf(
        buf,
        sizeof(buf),
        "%02lld:%02lld/%02lld:%02lld",
        posMinutes,
        posSeconds,
        lengthMinutes,
        lengthSeconds
    );
    return buf;
}