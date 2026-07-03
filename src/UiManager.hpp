#pragma once
#include "AudioManager.hpp"
#include "Events.hpp"
#include "InputManager.hpp"
#include "Level.hpp"
#include "WindowManager.hpp"
#include <imgui.h>
#include <numeric>

class UiManager {
  private:
    UiState currentState = UiState::MainMenu;

    ImFont* monocraftExtraSmall = nullptr;
    ImFont* monocraftSmall = nullptr;
    ImFont* monocraftMedium = nullptr;
    ImFont* monocraftLarge = nullptr;
    ImFont* monocraftExtraLarge = nullptr;
    ImFont* monocraftTitle = nullptr;

    const int MaxVolume = 100;

    ImGuiWindowFlags staticFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    void drawDebug(WindowManager& window, Entity* player, Camera* camera, InputManager& input);
    void drawMainMenu();
    void drawSettings(
        WindowManager& window, AudioManager& audio, InputManager& input, bool& showFanTriangulation
    );
    void drawPauseMenu();
    void drawLargeLogo();

  public:
    UiManager(AssetManager& assets, UiState startingState = UiState::MainMenu);

    void render(
        WindowManager& window,
        AudioManager& audio,
        InputManager& input,
        Level* level,
        bool& showFanTriangulation
    );

    void setState(UiState state);
    UiState getState() const;

    void runCancelEvent();

    bool showDebug = false;
};