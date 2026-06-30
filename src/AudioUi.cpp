#include "AudioUi.hpp"
#include <imgui.h>

static const int MaxVolume = 125;

void showAudioUi(AudioManager& audioManager) {
    ImGui::Begin("Audio");
    int masterVolume = audioManager.getVolume(AudioCategory::Master);
    int soundVolume = audioManager.getVolume(AudioCategory::Sounds);
    int musicVolume = audioManager.getVolume(AudioCategory::Music);
    if (ImGui::SliderInt("Master", &masterVolume, 0, MaxVolume)) {
        audioManager.setVolume(AudioCategory::Master, masterVolume);
    }
    if (ImGui::SliderInt("Sounds", &soundVolume, 0, MaxVolume)) {
        audioManager.setVolume(AudioCategory::Sounds, soundVolume);
    }
    if (ImGui::SliderInt("Music", &musicVolume, 0, MaxVolume)) {
        audioManager.setVolume(AudioCategory::Music, musicVolume);
    }
    ImGui::End();
}