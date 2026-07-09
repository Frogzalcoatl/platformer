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
    bool stateChangedThisFrame = false;

    ImFont* fontExtraSmall = nullptr;
    ImFont* fontSmall = nullptr;
    ImFont* fontMedium = nullptr;
    ImFont* fontLarge = nullptr;
    ImFont* fontExtraLarge = nullptr;
    ImFont* fontTitle = nullptr;

    const int MaxVolume = 100;

    ImGuiWindowFlags staticFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    ImGuiSliderFlags sliderFlags = ImGuiSliderFlags_NoInput;

    void drawDebug(
        WindowManager& window,
        Entity* player,
        Camera* camera,
        InputManager& input,
        Level* level,
        UiManager& uiManager
    );

    void drawLargeLogo();

    void drawMainMenu();

    void
    drawSettings(WindowManager& window, AudioManager& audio, InputManager& input, Level* level);

    void drawPauseMenu();

  public:
    UiManager(AssetManager& assets, UiState startingState = UiState::MainMenu);

    void draw(
        WindowManager& window,
        AudioManager& audio,
        InputManager& input,
        Level* level,
        UiManager& uiManager
    );

    void update();

    UiState getState() const;

    std::string getStateStr() const;

    void setState(UiState state);

    void runCancelEvent();

    void passInputToImGui(const GameEventTypes::Input& event);

    bool showDebug = false;
};