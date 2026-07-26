#include "gui/UiManager.hpp"

void UiManager::drawPlayerSourceSetup(WindowManager& window, InputManager& input) {
    setNextWindowSafeArea(window);
    ImGui::Begin(
        "Player Source Setup",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus
    );
    ImGui::PushFont(fontDoubleLarge);
    ImGui::Text("Player Source Setup:");
    ImGui::PopFont();
    ImGui::PushFont(fontLarge);
    ImGui::Dummy(ImVec2{0.f, 10.f * uiScale});
    ImGui::Text("Press any button to join!");
    static bool touchPlayerEnabled = input.isTouchPlayerEnabled(nullptr);
    if (input.hasTouchScreen()) {
        ImGui::Dummy(ImVec2{0.f, 25.f * uiScale});
        if (ImGui::Checkbox("Touch Player", &touchPlayerEnabled)) {
            if (touchPlayerEnabled) {
                input.enableTouchPlayer();
            } else {
                input.disableTouchPlayer();
            }
            touchPlayerEnabled = input.isTouchPlayerEnabled(nullptr);
        }
    }
    if (!ImGui::IsItemActive()) {
        touchPlayerEnabled = input.isTouchPlayerEnabled(nullptr);
    }
    ImGui::Dummy(ImVec2{0.f, 25.f * uiScale});
    const PlayerSources& playerSources = input.getPlayerSources();
    for (size_t i = 0; i < playerSources.size(); i++) {
        std::string childId = "Player " + std::to_string(i + 1);
        std::string sourceName;
        if (playerSources[i].has_value()) {
            sourceName = input.getSourceName(playerSources[i].value());
        } else {
            sourceName = "Empty";
        }
        ImGui::Text("%s: %s", childId.c_str(), sourceName.c_str());
        if (playerSources[i].has_value()) {
            ImGui::SameLine();
            std::string buttonId = "Remove##" + std::to_string(i + 1);
            if (ImGui::Button(buttonId.c_str()) && !playerSourceAddedThisFrame) {
                input.removePlayerSourceAtIndex(i);
            }
        }
        ImGui::Dummy(ImVec2{0.f, 50.f * uiScale});
    }
    if (ImGui::Button("Play", ImVec2{200.f * uiScale, 45.f * uiScale})) {
        const size_t playerSourceCount = input.getPlayerSourceCount();
        if (playerSourceCount > 0 && !playerSourceAddedThisFrame) {
            GameEvents::Push(GameEventTypes::ShouldDetectNewPlayerSources{false});
            GameEvents::Push(GameEventTypes::SetLevelName{LevelName::Test});
            // To make sure the ui screen is switched after the level is loaded.
            GameEvents::Push(GameEventTypes::SetUiState{UiState::Playing});
        } else {
            GameEvents::Push(
                GameEventTypes::SendNotification{"Must connect at least one valid player source"}
            );
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Back", ImVec2{200.f * uiScale, 45.f * uiScale})) {
        runCancelEvent();
    }
    ImGui::PopFont();
    applyTouchScroll();
    ImGui::End();
    setNextWindowFullscreen();
    ImGui::Begin(
        "PlayerSetupBackground",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoBringToFrontOnFocus
    );
    ImGui::End();
}