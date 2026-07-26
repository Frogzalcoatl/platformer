#include "gui/UiManager.hpp"

void UiManager::drawSettings(
    WindowManager& window,
    SettingsManager& settings,
    AudioManager& audio,
    InputManager& input,
    Level* level
) {
    setNextWindowSafeArea(window);
    const ImVec2 verticalSpacingDummy{0.f, 10.f * uiScale};
    const ImVec2 horizontalSpacingDummy{10.f * uiScale, 0.f};
    ImVec2 resetButtonSize{100 * uiScale, 30 * uiScale};
    if (ImGui::Begin(
            "Settings",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
        )) {
        ImGui::PushFont(fontLarge);
        if (ImGui::Button("Back")) {
            runCancelEvent();
        }
        ImGui::SameLine();
        ImGui::Dummy(horizontalSpacingDummy);
        ImGui::SameLine();
        if (ImGui::BeginTabBar("SettingsTabBar")) {
            ImGuiTabItemFlags displayTabFlags = ImGuiTabItemFlags_None;
            if (ImGui::IsWindowAppearing()) {
                displayTabFlags |= ImGuiTabItemFlags_SetSelected;
            }
            if (ImGui::BeginTabItem("Display", nullptr, displayTabFlags)) {
                ImGui::PushFont(fontDoubleLarge);
                ImGui::Text("Display");
                ImGui::PopFont();
                ImGui::Dummy(verticalSpacingDummy);
                fpsText(window);
                ImGui::Dummy(verticalSpacingDummy);
                bool vsync = window.isVsyncEnabled();
#ifndef SDL_PLATFORM_ANDROID
                if (ImGui::Checkbox("VSync", &vsync)) {
                    window.setVsync(vsync);
                    settings.setVsyncEnabled(vsync);
                    settings.setFpsUnlimited(window.getFpsUnlimited());
                    didEditSettings = true;
                }
                ImGui::Dummy(verticalSpacingDummy);
#endif
                bool fpsUnlimited = window.getFpsUnlimited();
                if (ImGui::Checkbox("FPS Unlimited", &fpsUnlimited)) {
                    window.setFpsUnlimited(fpsUnlimited);
                    settings.setFpsUnlimited(fpsUnlimited);
                    settings.setVsyncEnabled(window.isVsyncEnabled());
                    didEditSettings = true;
                }
                if (!vsync && !fpsUnlimited) {
                    ImGui::Dummy(verticalSpacingDummy);
                    static int tempFps = static_cast<int>(window.getTargetFps());
                    ImGui::SliderInt(
                        "Target FPS", &tempFps, 10, 300, "%d", ImGuiSliderFlags_NoInput
                    );
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        window.setTargetFps(static_cast<Uint64>(tempFps));
                        settings.setTargetFps(static_cast<unsigned int>(tempFps));
                        didEditSettings = true;
                    }
                    if (!ImGui::IsItemActive()) {
                        tempFps = static_cast<int>(window.getTargetFps());
                    }
                }
                ImGui::Dummy(verticalSpacingDummy);
                int activeIndex = 0;
                float minDiff = std::numeric_limits<float>::max();
                for (size_t i = 0; i < UiSizePresets.size(); i++) {
                    float diff = std::abs(UiSizePresets[i].scale - userPreferredScale);
                    if (diff < minDiff) {
                        minDiff = diff;
                        activeIndex = static_cast<int>(i);
                    }
                }
                static int tempIndex = activeIndex;
                if (ImGui::Button("Reset##ResetUIScale", resetButtonSize)) {
                    const Settings& defaultSettings = settings.getDefault();
                    userPreferredScale = UiSizePresets[defaultSettings.uiScale].scale;
                    tempIndex = static_cast<int>(defaultSettings.uiScale);
                    settings.setUiScale(defaultSettings.uiScale);
                    didEditSettings = true;
                }
                ImGui::SameLine();
                ImGui::Dummy(horizontalSpacingDummy);
                ImGui::SameLine();
                ImGui::SliderInt(
                    "UI Scale",
                    &tempIndex,
                    0,
                    static_cast<int>(UiSizePresets.size() - 1),
                    UiSizePresets[static_cast<size_t>(tempIndex)].name
                );
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    userPreferredScale = UiSizePresets[static_cast<size_t>(tempIndex)].scale;
                    activeIndex = tempIndex;
                    settings.setUiScale(static_cast<unsigned int>(tempIndex));
                    didEditSettings = true;
                }
                if (!ImGui::IsItemActive()) {
                    tempIndex = activeIndex;
                }
                if (level) {
                    ImGui::Dummy(verticalSpacingDummy);
                    ImGui::Checkbox("Show Hitboxes", &level->showHitBoxes);
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Audio")) {
                ImGui::PushFont(fontDoubleLarge);
                ImGui::Text("Audio");
                ImGui::PopFont();
                int masterVolume = static_cast<int>(audio.getVolume(AudioCategory::Master));
                int soundVolume = static_cast<int>(audio.getVolume(AudioCategory::Sounds));
                int musicVolume = static_cast<int>(audio.getVolume(AudioCategory::Music));
                float pitch = audio.getMusicPitch();
                ImGui::Dummy(verticalSpacingDummy);
                if (ImGui::Button("Reset##ResetMaster", resetButtonSize)) {
                    audio.setVolume(AudioCategory::Master, 100);
                    settings.setMasterVolume(100);
                    didEditSettings = true;
                }
                ImGui::SameLine();
                ImGui::Dummy(horizontalSpacingDummy);
                ImGui::SameLine();
                if (ImGui::SliderInt(
                        "Master", &masterVolume, 0, MaxVolume, "%d", ImGuiSliderFlags_NoInput
                    )) {
                    audio.setVolume(AudioCategory::Master, static_cast<unsigned int>(masterVolume));
                    settings.setMasterVolume(static_cast<unsigned int>(masterVolume));
                    didEditSettings = true;
                }
                ImGui::Dummy(verticalSpacingDummy);
                if (ImGui::Button("Reset##ResetSounds", resetButtonSize)) {
                    audio.setVolume(AudioCategory::Sounds, 100);
                    settings.setSoundsVolume(100);
                    didEditSettings = true;
                }
                ImGui::SameLine();
                ImGui::Dummy(horizontalSpacingDummy);
                ImGui::SameLine();
                if (ImGui::SliderInt(
                        "Sounds", &soundVolume, 0, MaxVolume, "%d", ImGuiSliderFlags_NoInput
                    )) {
                    audio.setVolume(AudioCategory::Sounds, static_cast<unsigned int>(soundVolume));
                    settings.setSoundsVolume(static_cast<unsigned int>(soundVolume));
                    didEditSettings = true;
                }
                ImGui::Dummy(verticalSpacingDummy);
                if (ImGui::Button("Reset##ResetMusic", resetButtonSize)) {
                    audio.setVolume(AudioCategory::Music, 100);
                    settings.setMusicVolume(100);
                    didEditSettings = true;
                }
                ImGui::SameLine();
                ImGui::Dummy(horizontalSpacingDummy);
                ImGui::SameLine();
                if (ImGui::SliderInt(
                        "Music", &musicVolume, 0, MaxVolume, "%d", ImGuiSliderFlags_NoInput
                    )) {
                    audio.setVolume(AudioCategory::Music, static_cast<unsigned int>(musicVolume));
                    settings.setMusicVolume(static_cast<unsigned int>(musicVolume));
                    didEditSettings = true;
                }
                ImGui::Dummy(verticalSpacingDummy);
                if (ImGui::Button("Reset##ResetMusicPitch", resetButtonSize)) {
                    audio.setMusicPitch(1.f);
                    didEditSettings = true;
                }
                ImGui::SameLine();
                ImGui::Dummy(horizontalSpacingDummy);
                ImGui::SameLine();
                if (ImGui::SliderFloat(
                        "Music Pitch", &pitch, 0.5f, 1.5f, "%.2f", ImGuiSliderFlags_NoInput
                    )) {
                    audio.setMusicPitch(pitch);
                    didEditSettings = true;
                }
                ImGui::Dummy(verticalSpacingDummy);
                ImGui::Text(
                    "Current Music: %s %s",
                    audio.getCurrentMusicName().c_str(),
                    audio.isMusicLooping() ? "(Looping)" : ""
                );
                ImGui::Text("Timestamp: %s", audio.formattedMusicTime().c_str());
                ImGui::Dummy(verticalSpacingDummy);
                if (ImGui::Button("Play Random Music")) {
                    audio.clearCurrentMusic();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Controls")) {
                ImGui::PushFont(fontDoubleLarge);
                ImGui::Text("Controls (Unfinished)");
                ImGui::PopFont();
                const ScancodeBindings& scancodeBidings = input.getScancodeBindings();
                for (size_t i = 0; i < static_cast<size_t>(InputVerb::VerbCount); i++) {
                    ImGui::Dummy(ImVec2{0.f, 25.f * uiScale});
                    std::string currentVerb = inputVerbToString(static_cast<InputVerb>(i)).c_str();
                    ImGui::Text("%s: ", currentVerb.c_str());
                    for (size_t j = 0; j < MaxBindsPerVerb; j++) {
                        std::string current = SDL_GetScancodeName(scancodeBidings[i][j].scancode);
                        current += "##" + currentVerb + "Index" + std::to_string(j);
                        ImGui::Button(current.c_str(), ImVec2{200.f * uiScale, 50.f * uiScale});
                        ImGui::SameLine();
                        ImGui::Dummy(ImVec2{10.f * uiScale, 0.f});
                        ImGui::SameLine();
                    }
                    ImGui::NewLine();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        applyTouchScroll();
        ImGui::PopFont();
    }
    ImGui::End();
    setNextWindowFullscreen();
    ImGui::Begin(
        "SettingsBackground",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoBringToFrontOnFocus
    );
    ImGui::End();
}