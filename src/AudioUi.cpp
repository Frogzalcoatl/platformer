#include "AudioUi.hpp"
#include <imgui.h>

static const int MaxVolume = 125;

void showAudioUi(AudioManager& audioManager) {
    ImGui::Begin("Audio");
    int masterVolume = audioManager.getVolume(AudioCategory::Master);
    int soundVolume = audioManager.getVolume(AudioCategory::Sounds);
    int musicVolume = audioManager.getVolume(AudioCategory::Music);
    float pitch = audioManager.getMusicPitch();
    if (ImGui::SmallButton("Reset##ResetMaster")) {
        audioManager.setVolume(AudioCategory::Master, 100);
    }
    ImGui::SameLine();
    if (ImGui::SliderInt("Master", &masterVolume, 0, MaxVolume)) {
        audioManager.setVolume(AudioCategory::Master, masterVolume);
    }
    if (ImGui::SmallButton("Reset##ResetSounds")) {
        audioManager.setVolume(AudioCategory::Sounds, 100);
    }
    ImGui::SameLine();
    if (ImGui::SliderInt("Sounds", &soundVolume, 0, MaxVolume)) {
        audioManager.setVolume(AudioCategory::Sounds, soundVolume);
    }
    if (ImGui::SmallButton("Reset##ResetMusic")) {
        audioManager.setVolume(AudioCategory::Music, 100);
    }
    ImGui::SameLine();
    if (ImGui::SliderInt("Music", &musicVolume, 0, MaxVolume)) {
        audioManager.setVolume(AudioCategory::Music, musicVolume);
    }
    if (ImGui::SmallButton("Reset##ResetMusicPitch")) {
        audioManager.setMusicPitch(1.f);
    }
    ImGui::SameLine();
    if (ImGui::SliderFloat("Music Pitch", &pitch, 0.25f, 2.f, "%.2f")) {
        audioManager.setMusicPitch(pitch);
    }
    ImGui::Dummy(ImVec2{1.f, 1.f});
    ImGui::Text(
        "Current Music: %s %s",
        audioManager.getCurrentMusicName(),
        audioManager.isMusicLooping() ? "(Looping)" : ""
    );
    ImGui::Text("Timestamp: %s", audioManager.formattedMusicTime().c_str());
    ImGui::End();
}