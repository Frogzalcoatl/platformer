#pragma once
#include "AudioManager.hpp"
#include "Events.hpp"
#include "InputManager.hpp"
#include "Level.hpp"
#include "SettingsManager.hpp"
#include "TouchController.hpp"
#include "WindowManager.hpp"
#include <imgui.h>
#include <memory>
#include <numeric>

struct UiSizePreset {
    float scale;
    const char* name;
};

class UiManager {
  private:
    UiState currentState = UiState::MainMenu;
    bool stateChangedThisFrame = false;
    bool playerSourceAddedThisFrame = false; // temporary fix probably

    const std::array<UiSizePreset, 6> UiSizePresets = {
        {{0.5f, "Extra Small"},
         {1.0f, "Small"},
         {1.5f, "Normal"},
         {2.0f, "Large"},
         {2.5f, "Extra Large"},
         {3.0f, "Ginormous"}}
    };

    float uiScale = 1.f;
    float userPreferredScale = 1.5f;

    ImGuiStyle defaultStyle;
    float logoHeight = 0.f;
    float logoTopPadding = 0.f;

    void updateActiveScale(WindowManager& window);
    void updateStyleScale(float scale);

    ImFont* fontSmall = nullptr;
    ImFont* fontMedium = nullptr;
    ImFont* fontLarge = nullptr;
    ImFont* fontDoubleLarge = nullptr;
    ImFont* fontTripleLarge = nullptr;
    ImFont* fontTitle = nullptr;

    const int MaxVolume = 100;

    ImGuiWindowFlags staticFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoSavedSettings;

    ImGuiSliderFlags sliderFlags = ImGuiSliderFlags_NoInput;

    std::unique_ptr<TouchController> touchController = nullptr;

    void applyTouchScroll();

    void setNextWindowFullscreen();
    void setNextWindowSafeArea(WindowManager& window);
    void setNextWindowYOnlySafeArea(WindowManager& window);

    void drawLargeLogo(WindowManager& window, float menuHeight);

    void fpsText(WindowManager& window);

    void drawDebug(
        WindowManager& window, Entity* player, Camera* camera, InputManager& input, Level* level
    );

    void drawMainMenu(WindowManager& window);

    void drawSettings(
        WindowManager& window,
        SettingsManager& settings,
        AudioManager& audio,
        InputManager& input,
        Level* level
    );

    void drawPlayerSourceSetup(WindowManager& window, InputManager& input);

    void drawPauseMenu(WindowManager& window);

  public:
    UiManager(AssetManager& assets, UiState startingState = UiState::MainMenu);

    bool showDebug = false;

    void draw(
        WindowManager& window,
        SettingsManager& settings,
        AudioManager& audio,
        InputManager& input,
        Level* level
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

    void enableTouchController(Entity& entity);

    void disableTouchController();

    int getFreeFingerCount() const;

    void setUserPreferredScale(size_t scaleIndex);

    size_t getUserPreferredScale() const;
};