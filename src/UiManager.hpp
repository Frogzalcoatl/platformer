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
    bool playerSourceAddedThisFrame = false; // temporary fix probably

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

    void setNextWindowFullscreen();

    void drawLargeLogo();

    void fpsText(WindowManager& window);

    void drawDebug(
        WindowManager& window,
        Entity* player,
        Camera* camera,
        InputManager& input,
        Level* level,
        UiManager& uiManager
    );

    void drawMainMenu();

    void
    drawSettings(WindowManager& window, AudioManager& audio, InputManager& input, Level* level);

    void drawPlayerSourceSetup(InputManager& input);

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

    void setPlayerSourceAddedThisFrame(bool value) {
        playerSourceAddedThisFrame = value;
    }
    bool isPlayerSourceAddedThisFrame() const {
        return playerSourceAddedThisFrame;
    }

    bool showDebug = false;
};