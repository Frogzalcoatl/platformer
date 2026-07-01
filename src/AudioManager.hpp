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
    MIX_Mixer* mixerDevice;
    MIX_Track* soundTracks[SoundTrackCount];
    MIX_Track* musicTrack;
    MIX_Audio* currentMusic = nullptr;
    float currentMusicVolume;
    std::string currentMusicName = "";
    const char* SoundTag = "Sounds";
    const char* MusicTag = "Music";
    std::array<MIX_Audio*, static_cast<size_t>(GameAssets::Sounds::SoundsCount)> loadedSounds;
    AssetManager* assets;
    std::array<float, static_cast<size_t>(AudioCategory::AudioCategoryCount)> volumeMultipliers;

  public:
    AudioManager(AssetManager& assets);
    ~AudioManager();

    void playSound(GameAssets::Sounds soundId, unsigned int volume = 100, float pitch = 1.f);
    void playMusic(
        GameAssets::Music musicId, unsigned int volume = 100, float pitch = 1.f, bool loop = false
    );
    void setVolume(AudioCategory category, unsigned int volume);
    unsigned int getVolume(AudioCategory category);
};