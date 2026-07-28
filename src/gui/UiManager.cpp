#include "gui/UiManager.hpp"
#include <array>
#include <cmath>
#include <format>
#include <imgui_internal.h>
#include <limits>

UiManager::UiManager(AssetManager& assets, UiState startingState) : currentState(startingState) {
    fontSmall = assets.getImGuiFont(AssetPaths::Fonts::Consolas, 12.f);
    fontMedium = assets.getImGuiFont(AssetPaths::Fonts::Consolas, 18.f);
    fontLarge = assets.getImGuiFont(AssetPaths::Fonts::Consolas, 24.f);
    fontDoubleLarge = assets.getImGuiFont(AssetPaths::Fonts::Consolas, 36.f);
    fontTripleLarge = assets.getImGuiFont(AssetPaths::Fonts::Consolas, 48.f);
    fontTitle = assets.getImGuiFont(AssetPaths::Fonts::Consolas, 128.f);
    defaultStyle = ImGui::GetStyle();
}

void UiManager::update(
    WindowManager& window,
    SettingsManager& settings,
    AudioManager& audio,
    InputManager& input,
    Level* level
) {
    stateChangedThisFrame = false;
    draw(window, settings, audio, input, level);
}

UiState UiManager::getState() const {
    return currentState;
}

std::string UiManager::getStateStr() const {
    switch (currentState) {
    case UiState::MainMenu:
        return "Main Menu";
    case UiState::Settings:
        return "Settings";
    case UiState::PlayerSourceSetup:
        return "Player Source Setup";
    case UiState::Playing:
        return "Playing";
    case UiState::Paused:
        return "Paused";
    case UiState::PausedSettings:
        return "Paused (Settings)";
    default:
        return "Invalid";
    }
}

void UiManager::setState(UiState newState) {
    if (newState >= UiState::UiStateCount || stateChangedThisFrame || newState == currentState) {
        return;
    }
    UiState previousState = currentState;
    currentState = newState;
    stateChangedThisFrame = true;
    if (previousState == UiState::PlayerSourceSetup) {
        // Switching off setup screen
        GameEvents::Push(GameEventTypes::ShouldDetectNewPlayerSources{false});
    }
    if (newState == UiState::PlayerSourceSetup) {
        // Switching to player source setup screen
        GameEvents::Push(GameEventTypes::ShouldDetectNewPlayerSources{true});
    }
    if (previousState == UiState::Settings && didEditSettings) {
        GameEvents::Push(GameEventTypes::SaveUserData{UserDataTypes::Settings});
        didEditSettings = false;
    }
    ImGuiIO& io = ImGui::GetIO();
    if (newState == UiState::Playing) {
        io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
    } else {
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    }
}

void UiManager::runCancelEvent() {
    switch (currentState) {
    case UiState::Settings:
        setState(UiState::MainMenu);
        break;
    case UiState::PlayerSourceSetup:
        setState(UiState::MainMenu);
        break;
    case UiState::Playing:
        setState(UiState::Paused);
        break;
    case UiState::Paused:
        setState(UiState::Playing);
        break;
    case UiState::PausedSettings:
        setState(UiState::Paused);
        break;
    default:
        break;
    }
}

void UiManager::toggleDebug() {
    if (std::find(debugVisibleIn.begin(), debugVisibleIn.end(), currentState) !=
        debugVisibleIn.end()) {
        showDebug = !showDebug;
    }
}

void UiManager::passInputToImGui(const GameEventTypes::Input& event) {
    ImGuiIO& io = ImGui::GetIO();
    ImGuiKey imguiKey = ImGuiKey_None;
    if (event.sourceInfo.type == InputType::Keyboard) {
        switch (event.verb) {
        case InputVerb::Up:
            imguiKey = ImGuiKey_UpArrow;
            break;
        case InputVerb::Down:
            imguiKey = ImGuiKey_DownArrow;
            break;
        case InputVerb::Left:
            imguiKey = ImGuiKey_LeftArrow;
            break;
        case InputVerb::Right:
            imguiKey = ImGuiKey_RightArrow;
            break;
        default:
            break;
        }
    }
    if (imguiKey != ImGuiKey_None) {
        io.AddKeyEvent(imguiKey, event.state == InputState::Pressed);
    }
}

void UiManager::enableTouchController(Entity& entity) {
    touchController = std::make_unique<TouchController>(entity);
}

void UiManager::disableTouchController() {
    touchController.reset();
}

int UiManager::getFreeFingerCount() const {
    if (touchController) {
        return touchController->getFreeFingerCount();
    } else {
        return 999;
    }
}

void UiManager::setScaleIndex(size_t scaleIndex) {
    const size_t MaxScaleIndex = UiSizePresets.size() - 1;
    if (scaleIndex > MaxScaleIndex) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Clamping user preferred scale from %zu to %zu",
            scaleIndex,
            MaxScaleIndex
        );
        scaleIndex = MaxScaleIndex;
    }
    userPreferredScale = UiSizePresets[scaleIndex].scale;
    SDL_Log("User preferred UI scale set to %zu", scaleIndex);
}

size_t UiManager::getScaleIndex() const {
    const float epsilon = 0.001f;
    for (size_t i = 0; i < UiSizePresets.size(); i++) {
        if (std::abs(UiSizePresets[i].scale - userPreferredScale) < epsilon) {
            return i;
        }
    }
    return 2; // fallback
}

void UiManager::draw(
    WindowManager& window,
    SettingsManager& settings,
    AudioManager& audio,
    InputManager& input,
    Level* level
) {
    updateActiveScale(window);
    Entity* playerEntity = nullptr;
    Camera* camera = nullptr;
    if (level) {
        const std::vector<Player>& players = level->getPlayers();
        if (!players.empty()) {
            EntityController* entityController = players.begin()->controller.get();
            if (entityController) {
                playerEntity = entityController->getEntity();
            }
        }
        camera = level->getCamera();
    }
    if (showDebug && std::find(debugVisibleIn.begin(), debugVisibleIn.end(), currentState) !=
                         debugVisibleIn.end()) {
        drawDebug(window, playerEntity, camera, input, level);
    }
    switch (currentState) {
    case UiState::MainMenu: {
        drawMainMenu(window);
    }; break;
    case UiState::Settings: {
        drawSettings(window, settings, audio, input, level);
    }; break;
    case UiState::PlayerSourceSetup: {
        drawPlayerSourceSetup(window, input);
    }; break;
    case UiState::Paused: {
        drawPauseMenu(window);
    }; break;
    case UiState::PausedSettings: {
        drawSettings(window, settings, audio, input, level);
    }; break;
    default:
        break;
    }
    if (touchController && currentState == UiState::Playing) {
        touchController->draw(window, uiScale);
    }
    playerSourceAddedThisFrame = false;
    itemActiveThisFrame = ImGui::IsAnyItemActive();
}

void UiManager::drawLargeLogo(WindowManager& window, float menuHeight) {
    SDL_Rect safeArea = window.getSafeArea();
    SDL_FRect safeAreaF{
        static_cast<float>(safeArea.x),
        static_cast<float>(safeArea.y),
        static_cast<float>(safeArea.w),
        static_cast<float>(safeArea.h)
    };
    float absoluteCenterX = static_cast<float>(window.getSize().x) * 0.5f;
    float idealPadding = 5.f;
    float logoMenuSpacing = 5.f * uiScale;
    float totalRequiredHeight = logoHeight + logoMenuSpacing + menuHeight;
    float maxAllowedPadding = safeAreaF.h - totalRequiredHeight;
    float actualLogoTopPadding =
        (maxAllowedPadding < idealPadding) ? maxAllowedPadding : idealPadding;
    if (actualLogoTopPadding < 0.f) {
        actualLogoTopPadding = 0.f;
    }
    logoTopPadding = actualLogoTopPadding;
    ImGui::SetNextWindowPos(
        ImVec2{absoluteCenterX, safeAreaF.y + logoTopPadding}, ImGuiCond_Always, ImVec2{0.5f, 0.f}
    );
    if (ImGui::Begin(
            "Main Menu Title",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing
        )) {
        ImGui::PushFont(fontTitle);
        ImGui::Text("Platformer");
        ImGui::PopFont();
        logoHeight = ImGui::GetWindowSize().y;
    }
    ImGui::End();
}

void UiManager::fpsText(WindowManager& window) {
    float fps = ImGui::GetIO().Framerate;
    std::string text;
    if (fps >= 1000) {
        text += std::format("{:0{}.{}f}", fps, 7, 1);
    } else {
        text += std::format("{:.1f}", fps);
    }
    if (!window.isVsyncEnabled() && !window.getFpsUnlimited()) {
        text += "/" + window.targetFpsStr();
    }
    text += " FPS (" + std::format("{:.3f}", 1000.f / fps) + " ms/frame)";
    ImGui::Text("%s", text.c_str());
}

void UiManager::applyClickSounds(
    std::string_view soundRelativePath, unsigned int volume, float pitch
) {
    if (!itemActiveThisFrame && ImGui::IsItemActivated()) {
        itemActiveThisFrame = true;
        GameEvents::Push(GameEventTypes::PlaySound{soundRelativePath, volume, pitch});
    }
}

void UiManager::applyHoverSounds(
    std::string_view soundRelativePath, unsigned int volume, float pitch
) {
    // Got idea to use item ids like this from ai.
    // Before was just using a simple boolean like in applyClickSounds.
    // but that meant hover sounds could only occur if no item was hovered last frame.
    // So if i went from hovering one item in frame 1 to another in frame 2, no sound would trigger
    // Also using func below from imgui internal prevents me from having to pass const char* ids in
    // this func.
    ImGuiID currentItemId = ImGui::GetItemID();
    if (ImGui::IsItemHovered()) {
        if (lastHoveredId != currentItemId) {
            lastHoveredId = currentItemId;
            GameEvents::Push(GameEventTypes::PlaySound{soundRelativePath, volume, pitch});
        }
    } else if (lastHoveredId == currentItemId) {
        lastHoveredId = 0;
    }
}

void UiManager::applyEditSounds(
    std::string_view soundRelativePath, unsigned int volume, float pitch
) {
    if (ImGui::IsItemEdited()) {
        GameEvents::Push(GameEventTypes::PlaySound{soundRelativePath, volume, pitch});
    }
}

void UiManager::applyTouchScroll() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.MouseSource != ImGuiMouseSource_TouchScreen) {
        return;
    }
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        ImVec2 delta = io.MouseDelta;
        ImGui::SetScrollY(ImGui::GetScrollY() - delta.y);
        ImGui::SetScrollX(ImGui::GetScrollX() - delta.x);
    }
}

void UiManager::updateActiveScale(WindowManager& window) {
    SDL_Rect safeArea = window.getSafeArea();
    const float baseMinWidth = 480.f;
    const float baseMinHeight = 540.f;
    const float baseDiagonal =
        std::sqrt(baseMinWidth * baseMinWidth + baseMinHeight * baseMinHeight);
    float currentDiagonal =
        std::sqrt(static_cast<float>(safeArea.w * safeArea.w + safeArea.h * safeArea.h));
    float maxSafeScale = currentDiagonal / baseDiagonal;
    const float maxMenuWidth = 320.f;
    const float maxMenuHeight = 350.f;

    float fitScaleW = static_cast<float>(safeArea.w) / maxMenuWidth;
    float fitScaleH = static_cast<float>(safeArea.h) / maxMenuHeight;
    float absoluteMaxScale = (fitScaleW < fitScaleH) ? fitScaleW : fitScaleH;
    if (maxSafeScale > absoluteMaxScale) {
        maxSafeScale = absoluteMaxScale;
    }
    if (maxSafeScale < 0.25f) {
        maxSafeScale = 0.25f;
    }
    float targetScale = (userPreferredScale < maxSafeScale) ? userPreferredScale : maxSafeScale;
    if (targetScale != uiScale) {
        updateStyleScale(targetScale);
    }
}

void UiManager::updateStyleScale(float scale) {
    uiScale = scale;
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = scale;
    ImGuiStyle& style = ImGui::GetStyle();
    style = defaultStyle;
    style.ScaleAllSizes(scale);
    setNextWindowFullscreen();
    ImGui::Begin(
        "SettingsBackdrop",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav
    );
    ImGui::End();
}

void UiManager::setNextWindowFullscreen() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
}

void UiManager::setNextWindowSafeArea(WindowManager& window) {
    SDL_Rect safeArea = window.getSafeArea();
    ImGui::SetNextWindowPos(ImVec2{static_cast<float>(safeArea.x), static_cast<float>(safeArea.y)});
    ImGui::SetNextWindowSize(
        ImVec2{static_cast<float>(safeArea.w), static_cast<float>(safeArea.h)}
    );
}

void UiManager::setNextWindowYOnlySafeArea(WindowManager& window) {
    SDL_Rect safeArea = window.getSafeArea();
    WindowVec2 windowSize = window.getSize();
    ImGui::SetNextWindowPos(ImVec2{0.f, static_cast<float>(safeArea.y)});
    ImGui::SetNextWindowSize(
        ImVec2{static_cast<float>(windowSize.x), static_cast<float>(safeArea.h)}
    );
}