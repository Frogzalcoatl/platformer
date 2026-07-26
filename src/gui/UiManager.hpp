#pragma once
#include "gui/TouchController.hpp"
#include "platformer/GameEvents.hpp"
#include "platformer/Level.hpp"
#include "system/AudioManager.hpp"
#include "system/InputManager.hpp"
#include "system/WindowManager.hpp"
#include "user-data/SettingsManager.hpp"
#include <array>
#include <imgui.h>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

struct UiSizePreset {
    float scale;
    const char* name;
};

class UiManager {
  public:
    UiManager(AssetManager& assets, UiState startingState = UiState::MainMenu);

    void update(
        WindowManager& window,
        SettingsManager& settings,
        AudioManager& audio,
        InputManager& input,
        Level* level
    );

    UiState getState() const;
    std::string getStateStr() const;
    void setState(UiState state);
    void runCancelEvent();
    void toggleDebug();

    void passInputToImGui(const GameEventTypes::Input& event);
    void enableTouchController(Entity& entity);
    void disableTouchController();
    int getFreeFingerCount() const;

    void setScaleIndex(size_t scaleIndex);
    size_t getScaleIndex() const;
    float getActualScale() const {
        return uiScale;
    }

    void setPlayerSourceAddedThisFrame(bool value) {
        playerSourceAddedThisFrame = value;
    }
    bool isPlayerSourceAddedThisFrame() const {
        return playerSourceAddedThisFrame;
    }

  private:
    void draw(
        WindowManager& window,
        SettingsManager& settings,
        AudioManager& audio,
        InputManager& input,
        Level* level
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
    void drawDebug(
        WindowManager& window, Entity* player, Camera* camera, InputManager& input, Level* level
    );
    void drawLargeLogo(WindowManager& window, float menuHeight);
    void fpsText(WindowManager& window);

    void handleSounds();
    void applyTouchScroll();
    void updateActiveScale(WindowManager& window);
    void updateStyleScale(float scale);
    void setNextWindowFullscreen();
    void setNextWindowSafeArea(WindowManager& window);
    void setNextWindowYOnlySafeArea(WindowManager& window);

    UiState currentState = UiState::MainMenu;
    bool stateChangedThisFrame = false;
    bool playerSourceAddedThisFrame = false;

    bool itemHoveredThisFrame = false;
    bool itemActiveThisFrame = false;
    bool didEditSettings = false;

    float uiScale = 1.f;
    float userPreferredScale = 1.5f;
    ImGuiStyle defaultStyle;

    float logoHeight = 0.f;
    float logoTopPadding = 0.f;

    ImFont* fontSmall = nullptr;
    ImFont* fontMedium = nullptr;
    ImFont* fontLarge = nullptr;
    ImFont* fontDoubleLarge = nullptr;
    ImFont* fontTripleLarge = nullptr;
    ImFont* fontTitle = nullptr;

    const int MaxVolume = 100;

    std::unique_ptr<TouchController> touchController = nullptr;

    bool showDebug = false;
    const std::vector<UiState> debugVisibleIn = {
        UiState::MainMenu, UiState::Paused, UiState::Playing
    };

    const std::array<UiSizePreset, 6> UiSizePresets = {
        {{0.5f, "Extra Small"},
         {1.0f, "Small"},
         {1.5f, "Normal"},
         {2.0f, "Large"},
         {2.5f, "Extra Large"},
         {3.0f, "Ginormous"}}
    };
};