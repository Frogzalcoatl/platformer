#pragma once
#include "AssetManager.hpp"
#include "Events.hpp"
#include <SDL3_mixer/SDL_mixer.h>
#include <array>
#include <memory>
#include <string>

inline constexpr size_t SoundTrackCount = 8;

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
using UniqueMixer = std::unique_ptr<MIX_Mixer, MIX_Mixer_Deleter>;
using UniqueTrack = std::unique_ptr<MIX_Track, MIX_Track_Deleter>;

class AudioManager {
  private:
    AssetManager* assetManager;
    UniqueMixer mixerDevice;
    std::array<const char*, static_cast<size_t>(AudioCategory::AudioCategoryCount)> TagNames = {
        "Master", "Sounds", "Music"
    };
    std::array<float, static_cast<size_t>(AudioCategory::AudioCategoryCount)> tagGain = {};
    std::array<UniqueTrack, SoundTrackCount> soundTracks = {};
    UniqueTrack musicTrack;
    float currentMusicVolume =
        1.f; // Separate volume multiplier, based on volume passed into playMusic func
    MIX_Audio* currentMusic = nullptr;
    std::filesystem::path currentMusicRelativePath;
    std::string currentMusicName = "";

  public:
    AudioManager(AssetManager& assetManager);

    bool playSound(std::string_view relativePath, unsigned int volume = 100, float pitch = 1.f);

    bool playMusic(
        std::string_view relativePath,
        unsigned int volume = 100,
        float pitch = 1.f,
        bool loop = false
    );

    MIX_Mixer* getMixerDevice() const {
        return mixerDevice.get();
    }

    unsigned int getVolume(AudioCategory category);

    void setVolume(AudioCategory category, unsigned int volume);

    void pauseCategory(AudioCategory category);

    void unpauseCategory(AudioCategory category);

    void clearCurrentMusic();

    bool isMusicPlaying() const;

    bool isMusicLooping() const;

    float getMusicPitch() const;

    void setMusicPitch(float pitch);

    std::string getCurrentMusicName() const;

    Sint64 getMusicPlaybackPosition() const; // In seconds

    Sint64 getMusicTimeRemaining() const; // In seconds

    Sint64 getMusicLength() const; // In seconds

    std::string formattedMusicTime() const; // MM:SS
};