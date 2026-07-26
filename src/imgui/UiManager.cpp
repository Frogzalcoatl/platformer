#include "imgui/UiManager.hpp"
#include <array>
#include <cmath>
#include <format>
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

void UiManager::handleSounds() {
    bool itemHoveredLastFrame = itemHoveredThisFrame;
    itemHoveredThisFrame = ImGui::IsAnyItemHovered();
    if (!itemHoveredLastFrame && itemHoveredThisFrame) {
        GameEvents::Push(GameEventTypes::PlaySound{AssetPaths::Sounds::Hover});
    }
    bool itemActiveLastFrame = itemActiveThisFrame;
    itemActiveThisFrame = ImGui::IsAnyItemActive();
    if (!itemActiveLastFrame && itemActiveThisFrame) {
        GameEvents::Push(GameEventTypes::PlaySound{AssetPaths::Sounds::Click});
    }
}

void UiManager::toggleDebug() {
    if (std::find(debugVisibleIn.begin(), debugVisibleIn.end(), currentState) !=
        debugVisibleIn.end()) {
        showDebug = !showDebug;
    }
}

void UiManager::draw(
    WindowManager& window,
    SettingsManager& settings,
    AudioManager& audio,
    InputManager& input,
    Level* level
) {
    handleSounds();
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
}

void UiManager::update() {
    stateChangedThisFrame = false;
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
        staticFlags | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav
    );
    ImGui::End();
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
            staticFlags | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing
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

void UiManager::drawDebug(
    WindowManager& window, Entity* playerEntity, Camera* camera, InputManager& input, Level* level
) {
    ImGui::PushFont(fontSmall);
    SDL_Rect safeArea = window.getSafeArea();
    ImGui::SetNextWindowPos(
        ImVec2{static_cast<float>(safeArea.x), static_cast<float>(safeArea.y)}, ImGuiCond_Always
    );
    if (ImGui::Begin("Debug Menu", nullptr, staticFlags | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Window:");
        WindowVec2 windowSize = window.getSize();
        ImGui::Text("Size: %d, %d", windowSize.x, windowSize.y);
        ImGui::Text("Framerate:");
        ImGui::SameLine();
        fpsText(window);
        ImGui::Dummy(ImVec2{1.f, 1.f});
        ImGui::Text("\nUI State: %s", getStateStr().c_str());
        ImGui::Dummy(ImVec2{1.f, 1.f});
        if (level) {
            std::string_view levelName = level->getName();
            LevelDimensions levelSize = level->getSize();
            LevelDrawInfo drawInfo = level->drawnLastFrame();
            size_t tileCount = level->getTileCount();
            size_t entitiesCount = level->getEntities().size();
            ImGui::Text("\nLevel:");
            ImGui::Text(
                // %.*s tells the func to read exactly N characters, preventing it from running past
                // the end of a string_view
                "Name: \"%.*s\"\nSize: (%zu, %zu)\nTiles Drawn: %zu/%zu\nEntities Drawn: %zu/%zu",
                static_cast<int>(levelName.length()),
                levelName.data(),
                levelSize.width,
                levelSize.height,
                drawInfo.tiles,
                tileCount,
                drawInfo.entities,
                entitiesCount
            );
            ImGui::Dummy(ImVec2{1.f, 1.f});
        }
        if (playerEntity) {
            b2Vec2 position = b2Body_GetPosition(playerEntity->getBodyId());
            b2Vec2 velocity = b2Body_GetLinearVelocity(playerEntity->getBodyId());
            ImGui::Text("\nPlayer 1:");
            ImGui::Text(
                "Position: %.2f, %.2f\nVelocity: %.2f, %.2f",
                position.x,
                position.y,
                velocity.x,
                velocity.y
            );
            ImGui::Dummy(ImVec2{1.f, 1.f});
        }
        if (camera) {
            const b2Vec2 offsetWorld = camera->getOffsetWorld();
            const WindowVec2 offsetPixels = camera->getOffsetPixels();
            const b2Vec2 size = camera->getSize();
            const b2Vec2 safeAreaSize = camera->getSafeAreaSize();
            const b2Vec2 safeAreaValue = camera->getEntitySafeAreaValue();
            const b2Vec2 mouseWorldPos = camera->pixelPosToWorldPos(window.getMousePos());
            ImGui::Text("\nCamera:");
            ImGui::Text(
                "Offset Pixels: %d, %d\nOffset World: %.2f, %.2f\nSize World: %.2f, %.2f\nSafe Area Size World: %.2f, %.2f\nPlayer Ratio from Center: %.2f, %.2f\nMouse Position World: %.2f, %.2f",
                offsetPixels.x,
                offsetPixels.y,
                offsetWorld.x,
                offsetWorld.y,
                size.x,
                size.y,
                safeAreaSize.x,
                safeAreaSize.y,
                safeAreaValue.x,
                safeAreaValue.y,
                mouseWorldPos.x,
                mouseWorldPos.y
            );
            ImGui::Dummy(ImVec2{1.f, 1.f});
        }
        int sdlGamepadCount = input.sdlGamepadsDetected();
        ImGui::Text("\nSDL Gamepads Detected: %d", sdlGamepadCount);
        size_t playerSourceCount = input.getPlayerSourceCount();
        size_t maxPlayerSourceCount = input.getPlayerSources().size();
        ImGui::Text("Player Sources Connected: %zu/%zu", playerSourceCount, maxPlayerSourceCount);
        ImGui::PopFont();
    }
    ImGui::End();
}

void UiManager::drawMainMenu(WindowManager& window) {
    float menuHeight = 250.f * uiScale;
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
    if (ImGui::Begin("Main Menu", nullptr, staticFlags | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
        ImGui::PushFont(fontLarge);
        float verticalSpacing = 15.f * uiScale;
        float windowWidth = ImGui::GetWindowSize().x;
        float buttonWidth = 225.f * uiScale;
        float buttonHeight = 50.f * uiScale;
        float cursorX = (windowWidth - buttonWidth) * 0.5f;
        ImGui::SetCursorPosY(25.f * uiScale);
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Test Game", ImVec2{buttonWidth, buttonHeight})) {
            setState(UiState::PlayerSourceSetup);
        }
        ImGui::SetItemDefaultFocus();
        ImGui::Dummy(ImVec2(0, verticalSpacing));
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Settings", ImVec2{buttonWidth, buttonHeight})) {
            setState(UiState::Settings);
        }
        ImGui::Dummy(ImVec2(0, verticalSpacing));
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Quit", ImVec2{buttonWidth, buttonHeight})) {
            GameEvents::Push(GameEventTypes::CloseWindow{});
        }
        ImGui::PopFont();
    }
    ImGui::End();
}

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
                if (ImGui::Checkbox("VSync", &vsync)) {
                    window.setVsync(vsync);
                    settings.setVsyncEnabled(vsync);
                    settings.setFpsUnlimited(window.getFpsUnlimited());
                    didEditSettings = true;
                }
                ImGui::Dummy(verticalSpacingDummy);
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
                    ImGui::SliderInt("Target FPS", &tempFps, 10, 300, "%d", sliderFlags);
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
                if (ImGui::SliderInt("Master", &masterVolume, 0, MaxVolume, "%d", sliderFlags)) {
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
                if (ImGui::SliderInt("Sounds", &soundVolume, 0, MaxVolume, "%d", sliderFlags)) {
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
                if (ImGui::SliderInt("Music", &musicVolume, 0, MaxVolume, "%d", sliderFlags)) {
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
                if (ImGui::SliderFloat("Music Pitch", &pitch, 0.5f, 1.5f, "%.2f", sliderFlags)) {
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
        staticFlags | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoBringToFrontOnFocus
    );
    ImGui::End();
}

void UiManager::drawPlayerSourceSetup(WindowManager& window, InputManager& input) {
    setNextWindowSafeArea(window);
    ImGui::Begin(
        "Player Source Setup",
        nullptr,
        staticFlags | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus
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
        staticFlags | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoBringToFrontOnFocus
    );
    ImGui::End();
}

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
    if (ImGui::Begin("Pause Menu", nullptr, staticFlags | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
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
        ImGui::SetItemDefaultFocus();
        ImGui::Dummy(ImVec2(0, verticalSpacing));
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Settings", ImVec2{buttonWidth, buttonHeight})) {
            setState(UiState::PausedSettings);
        }
        ImGui::Dummy(ImVec2(0, verticalSpacing));
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Exit", ImVec2{buttonWidth, buttonHeight})) {
            GameEvents::Push(GameEventTypes::SetLevelName{LevelName::None});
            setState(UiState::MainMenu);
        }
        ImGui::PopFont();
    }
    ImGui::End();
}