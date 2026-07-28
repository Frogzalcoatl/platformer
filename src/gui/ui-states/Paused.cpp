#include "gui/UiManager.hpp"

void UiManager::drawPauseMenu(WindowManager& window) {
    float menuHeight = 325.f * uiScale;
    drawLargeLogo(window, menuHeight);
    SDL_Rect safeArea = window.getSafeArea();
    SDL_FRect safeAreaF{
        static_cast<float>(safeArea.x),
        static_cast<float>(safeArea.y),
        static_cast<float>(safeArea.w),
        static_cast<float>(safeArea.h)
    };
    WindowVec2 windowSize = window.getSize();
    ImVec2 windowSizeF{static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)};
    float absoluteCenterX = windowSizeF.x * 0.5f;
    ImVec2 menuSize = ImVec2{300.f * uiScale, menuHeight};
    float logoTop = safeAreaF.y + logoTopPadding;
    float logoBottom = logoTop + logoHeight;
    float logoMenuSpacing = 15.f * uiScale;
    float actualMenuTop = logoBottom + logoMenuSpacing;
    ImGui::SetNextWindowPos(
        ImVec2{absoluteCenterX, actualMenuTop}, ImGuiCond_Always, ImVec2{0.5f, 0.0f}
    );
    ImGui::SetNextWindowSize(menuSize);
    if (ImGui::Begin(
            "Pause Menu",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoBringToFrontOnFocus
        )) {
        ImGui::PushFont(fontLarge);
        float verticalSpacing = 15.f * uiScale;
        float windowWidth = ImGui::GetWindowSize().x;
        float buttonWidth = 225.f * uiScale;
        float buttonHeight = 50.f * uiScale;
        float cursorX = (windowWidth - buttonWidth) * 0.5f;
        ImGui::SetCursorPosY(25.f * uiScale);
        const char pausedText[] = "> Paused <";
        ImVec2 pauseTextSize = ImGui::CalcTextSize(pausedText);
        ImGui::SetCursorPosX((windowWidth - pauseTextSize.x) * 0.5f);
        ImGui::Text(pausedText);
        ImGui::Dummy(ImVec2(0, 50.f * uiScale));
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Resume", ImVec2{buttonWidth, buttonHeight})) {
            setState(UiState::Playing);
        }
        applyHoverSounds();
        applyClickSounds();
        ImGui::SetItemDefaultFocus();
        ImGui::Dummy(ImVec2(0, verticalSpacing));
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Settings", ImVec2{buttonWidth, buttonHeight})) {
            setState(UiState::PausedSettings);
        }
        applyHoverSounds();
        applyClickSounds();
        ImGui::Dummy(ImVec2(0, verticalSpacing));
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Exit", ImVec2{buttonWidth, buttonHeight})) {
            GameEvents::Push(GameEventTypes::SetLevelName{LevelName::None});
            setState(UiState::MainMenu);
        }
        applyHoverSounds();
        applyClickSounds();
        ImGui::PopFont();
    }
    ImGui::End();
}