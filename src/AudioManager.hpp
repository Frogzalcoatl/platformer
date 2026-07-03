#pragma once
#include "AssetManager.hpp"
#include <SDL3_mixer/SDL_mixer.h>
#include <array>
#include <string>

inline constexpr size_t SoundTrackCount = 8;

enum class AudioCategory : uint8_t {
    Master,
    Sounds,
    Music,
    AudioCategoryCount,
};

class AudioManager {
  private:
    AssetManager* assets;
    MIX_Mixer* mixerDevice;
    MIX_Track* soundTracks[SoundTrackCount];
    MIX_Track* musicTrack;
    const char* SoundTag = "Sounds";
    const char* MusicTag = "Music";
    std::array<MIX_Audio*, static_cast<size_t>(GameAssets::Sounds::SoundsCount)> loadedSounds;
    std::array<float, static_cast<size_t>(AudioCategory::AudioCategoryCount)> volumeMultipliers;
    MIX_Audio* currentMusic = nullptr;
    float currentMusicVolume =
        1.f; // Separate volume multiplier, based on volume passed into playMusic func
    const char* currentMusicName = "";

  public:
    AudioManager(AssetManager& assets);
    ~AudioManager();

    bool playSound(GameAssets::Sounds soundId, unsigned int volume = 100, float pitch = 1.f);
    bool playMusic(
        GameAssets::Music musicId, unsigned int volume = 100, float pitch = 1.f, bool loop = false
    );
    void setVolume(AudioCategory category, unsigned int volume);
    void setMusicPitch(float pitch);
    void clearCurrentMusic();
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