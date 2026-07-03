#pragma once
#include "AssetManager.hpp"
#include <SDL3_mixer/SDL_mixer.h>
#include <array>
#include <memory>
#include <string>

inline constexpr size_t SoundTrackCount = 8;

enum class AudioCategory : uint8_t {
    Master,
    Sounds,
    Music,
    AudioCategoryCount,
};

struct MIX_Mixer_Deleter {
    void operator()(MIX_Mixer* m) const {
        if (m) {
            MIX_DestroyMixer(m);
            SDL_Log("Destroyed SDL3 mixer device");
        }
    }
};
struct MIX_Track_Deleter {
    void operator()(MIX_Track* t) const {
        if (t) {
            MIX_DestroyTrack(t);
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
using UniqueMixer = std::unique_ptr<MIX_Mixer, MIX_Mixer_Deleter>;
using UniqueTrack = std::unique_ptr<MIX_Track, MIX_Track_Deleter>;
using UniqueAudio = std::unique_ptr<MIX_Audio, MIX_Audio_Deleter>;

class AudioManager {
  private:
    AssetManager* assets;
    UniqueMixer mixerDevice;
    std::array<UniqueTrack, SoundTrackCount> soundTracks;
    UniqueTrack musicTrack;
    std::array<const char*, static_cast<size_t>(AudioCategory::AudioCategoryCount)> TagNames = {
        "Master", "Sounds", "Music"
    };
    std::array<UniqueAudio, static_cast<size_t>(GameAssets::Sounds::SoundsCount)> loadedSounds;
    std::array<float, static_cast<size_t>(AudioCategory::AudioCategoryCount)> tagGain;
    float currentMusicVolume =
        1.f; // Separate volume multiplier, based on volume passed into playMusic func
    UniqueAudio currentMusic = nullptr;
    const char* currentMusicName = "";

  public:
    AudioManager(AssetManager& assets);

    bool playSound(GameAssets::Sounds soundId, unsigned int volume = 100, float pitch = 1.f);
    bool playMusic(
        GameAssets::Music musicId, unsigned int volume = 100, float pitch = 1.f, bool loop = false
    );
    void setVolume(AudioCategory category, unsigned int volume);
    void setMusicPitch(float pitch);
    void clearCurrentMusic();
    void pauseCategory(AudioCategory category);
    void unpauseCategory(AudioCategory category);
    unsigned int getVolume(AudioCategory category);
    const char* getCurrentMusicName() const;
    float getMusicPitch() const;
    bool isMusicLooping() const;
    bool isMusicPlaying() const;
    Sint64 getMusicPlaybackPosition() const; // In seconds
    Sint64 getMusicTimeRemaining() const;    // In seconds
    Sint64 getMusicLength() const;           // In seconds
    std::string formattedMusicTime() const;  // MM:SS
};