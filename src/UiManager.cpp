#include "UiManager.hpp"
#include "AssetPaths.hpp"

UiManager::UiManager(AssetManager& assets, UiState startingState) : currentState(startingState) {
    fontExtraSmall = assets.getImGuiFont(AssetPaths::Fonts::Consolas, 12.f);
    fontSmall = assets.getImGuiFont(AssetPaths::Fonts::Consolas, 18.f);
    fontMedium = assets.getImGuiFont(AssetPaths::Fonts::Consolas, 24.f);
    fontLarge = assets.getImGuiFont(AssetPaths::Fonts::Consolas, 36.f);
    fontExtraLarge = assets.getImGuiFont(AssetPaths::Fonts::Consolas, 48.f);
    fontTitle = assets.getImGuiFont(AssetPaths::Fonts::Consolas, 128.f);
}

void UiManager::draw(
    WindowManager& window,
    AudioManager& audio,
    InputManager& input,
    Level* level,
    UiManager& uiManager
) {
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
    if (showDebug) {
        drawDebug(window, playerEntity, camera, input, level, uiManager);
    }
    switch (currentState) {
    case UiState::MainMenu: {
        drawMainMenu();
    }; break;
    case UiState::Settings: {
        drawSettings(window, audio, input, level);
    }; break;
    case UiState::PlayerSourceSetup: {
        drawPlayerSourceSetup(input);
    }; break;
    case UiState::Paused: {
        drawPauseMenu();
    }; break;
    case UiState::PausedSettings: {
        drawSettings(window, audio, input, level);
    }; break;
    default:
        break;
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
    if (currentState == UiState::PlayerSourceSetup) {
        // Switching off setup screen
        GameEvents::Push(GameEventTypes::ShouldDetectNewPlayerSources{false});
    }
    if (newState == UiState::PlayerSourceSetup) {
        // Switching to player source setup screen
        GameEvents::Push(GameEventTypes::ShouldDetectNewPlayerSources{true});
    }
    currentState = newState;
    stateChangedThisFrame = true;
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

void UiManager::setNextWindowFullscreen() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    ImVec2 menuPos = ImVec2{workPos.x + workSize.x * 0.5f, workPos.y + workSize.y * 0.5f};
    ImGui::SetNextWindowPos(menuPos, ImGuiCond_Always, ImVec2{0.5, 0.5});
    ImGui::SetNextWindowSize(workSize);
}

void UiManager::drawLargeLogo() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    ImVec2 centerPos = ImVec2{workPos.x + workSize.x * 0.5f, workPos.y + workSize.y * 0.5f};
    ImGui::SetNextWindowPos(ImVec2{centerPos.x, 50.f}, ImGuiCond_Always, ImVec2{0.5, 0.f});
    if (ImGui::Begin("Main Menu Title", nullptr, staticFlags)) {
        ImGui::PushFont(fontTitle);
        ImGui::Text("Platformer");
        ImGui::PopFont();
    }
    ImGui::End();
}

void UiManager::fpsText(WindowManager& window) {
    float fps = ImGui::GetIO().Framerate;
    if (fps >= 1000) {
        ImGui::Text(
            "%07.1f/%s FPS (%.3f ms/frame)", fps, window.targetFpsStr().c_str(), 1000.0f / fps
        );
    } else {
        ImGui::Text(
            "%.1f/%s FPS (%.3f ms/frame)", fps, window.targetFpsStr().c_str(), 1000.0f / fps
        );
    }
}

void UiManager::drawDebug(
    WindowManager& window,
    Entity* playerEntity,
    Camera* camera,
    InputManager& input,
    Level* level,
    UiManager& uiManager
) {
    ImGui::PushFont(fontSmall);
    ImGui::SetNextWindowPos(ImVec2{0.f, 0.f}, ImGuiCond_Always);
    if (ImGui::Begin("Debug Menu", nullptr, staticFlags | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Window:");
        WindowVec2 windowSize = window.getSize();
        ImGui::Text("Size: %d, %d", windowSize.x, windowSize.y);
        ImGui::Text("Framerate:");
        ImGui::SameLine();
        fpsText(window);
        ImGui::Dummy(ImVec2{1.f, 1.f});
        ImGui::Text("\nUI State: %s", uiManager.getStateStr().c_str());
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

void UiManager::drawMainMenu() {
    drawLargeLogo();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    ImVec2 centerPos = ImVec2{workPos.x + workSize.x * 0.5f, workPos.y + workSize.y * 0.5f};
    ImVec2 menuSize = ImVec2{425.f, 375.f};
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2{0.5, 0.5});
    ImGui::SetNextWindowSize(menuSize);
    if (ImGui::Begin("Main Menu", nullptr, staticFlags)) {
        ImGui::PushFont(fontLarge);
        float verticalSpacing = 15.f;
        float windowWidth = ImGui::GetWindowSize().x;
        float buttonWidth = 350.f;
        float buttonHeight = 75.f;
        float cursorX = (windowWidth - buttonWidth) * 0.5f;
        ImGui::SetCursorPosY(50.f);
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
    ImGui::SetWindowFocus("Main Menu");
    ImGui::End();
}

void UiManager::drawSettings(
    WindowManager& window, AudioManager& audio, InputManager& input, Level* level
) {
    setNextWindowFullscreen();
    const ImVec2 verticalSpacingDummy{0.f, 10.f};
    const ImVec2 horizontalSpacingDummy{10.f, 0.f};
    if (ImGui::Begin("Settings", nullptr, staticFlags)) {
        ImGui::PushFont(fontLarge);
        if (ImGui::Button("Back", ImVec2{100.f, 45.f})) {
            runCancelEvent();
        }
        ImGui::SameLine();
        ImGui::Dummy(horizontalSpacingDummy);
        ImGui::SameLine();
        ImGui::PushFont(fontLarge);
        ImGui::BeginTabBar("SettingsTabBar");
        ImGuiTabItemFlags displayTabFlags = ImGuiTabItemFlags_None;
        if (ImGui::IsWindowAppearing()) {
            displayTabFlags |= ImGuiTabItemFlags_SetSelected;
        }
        if (ImGui::BeginTabItem("Display", nullptr, displayTabFlags)) {
            ImGui::Text("Display");
            ImGui::PushFont(fontMedium);
            ImGui::Dummy(verticalSpacingDummy);
            fpsText(window);
            ImGui::Dummy(verticalSpacingDummy);
            bool vsync = window.isVsyncEnabled();
            if (ImGui::Checkbox("VSync", &vsync)) {
                window.setVsync(vsync);
            }
            ImGui::Dummy(verticalSpacingDummy);
            bool fpsUnlimited = window.getFpsUnlimited();
            if (ImGui::Checkbox("FPS Unlimited", &fpsUnlimited)) {
                window.setFpsUnlimited(fpsUnlimited);
            }
            ImGui::Dummy(verticalSpacingDummy);
            if (!vsync && !fpsUnlimited) {
                static int tempFps = static_cast<int>(window.getTargetFps());
                ImGui::SliderInt("Target FPS", &tempFps, 10, 300, "%d", sliderFlags);
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    window.setTargetFps(tempFps);
                }
            }
            if (level) {
                ImGui::Dummy(verticalSpacingDummy);
                ImGui::Checkbox("Show Hitboxes", &level->showHitBoxes);
            }
            ImGui::PopFont();
            ImGui::EndTabItem();
        }
        ImGui::PopFont();
        ImGui::PushFont(fontLarge);
        if (ImGui::BeginTabItem("Audio")) {
            ImGui::Text("Audio");
            ImGui::PushFont(fontMedium);
            int masterVolume = audio.getVolume(AudioCategory::Master);
            int soundVolume = audio.getVolume(AudioCategory::Sounds);
            int musicVolume = audio.getVolume(AudioCategory::Music);
            float pitch = audio.getMusicPitch();
            ImVec2 resetButtonSize{100, 30};
            ImGui::Dummy(verticalSpacingDummy);
            if (ImGui::Button("Reset##ResetMaster", resetButtonSize)) {
                audio.setVolume(AudioCategory::Master, 100);
            }
            ImGui::SameLine();
            ImGui::Dummy(horizontalSpacingDummy);
            ImGui::SameLine();
            if (ImGui::SliderInt("Master", &masterVolume, 0, MaxVolume, "%d", sliderFlags)) {
                audio.setVolume(AudioCategory::Master, masterVolume);
            }
            ImGui::Dummy(verticalSpacingDummy);
            if (ImGui::Button("Reset##ResetSounds", resetButtonSize)) {
                audio.setVolume(AudioCategory::Sounds, 100);
            }
            ImGui::SameLine();
            ImGui::Dummy(horizontalSpacingDummy);
            ImGui::SameLine();
            if (ImGui::SliderInt("Sounds", &soundVolume, 0, MaxVolume, "%d", sliderFlags)) {
                audio.setVolume(AudioCategory::Sounds, soundVolume);
            }
            ImGui::Dummy(verticalSpacingDummy);
            if (ImGui::Button("Reset##ResetMusic", resetButtonSize)) {
                audio.setVolume(AudioCategory::Music, 100);
            }
            ImGui::SameLine();
            ImGui::Dummy(horizontalSpacingDummy);
            ImGui::SameLine();
            if (ImGui::SliderInt("Music", &musicVolume, 0, MaxVolume, "%d", sliderFlags)) {
                audio.setVolume(AudioCategory::Music, musicVolume);
            }
            ImGui::Dummy(verticalSpacingDummy);
            if (ImGui::Button("Reset##ResetMusicPitch", resetButtonSize)) {
                audio.setMusicPitch(1.f);
            }
            ImGui::SameLine();
            ImGui::Dummy(horizontalSpacingDummy);
            ImGui::SameLine();
            if (ImGui::SliderFloat("Music Pitch", &pitch, 0.5f, 1.5f, "%.2f", sliderFlags)) {
                audio.setMusicPitch(pitch);
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
            ImGui::PopFont();
            ImGui::EndTabItem();
        }
        ImGui::PopFont();
        ImGui::PushFont(fontLarge);
        if (ImGui::BeginTabItem("Controls")) {
            ImGui::Text("Controls (Unfinished)");
            ImGui::PushFont(fontMedium);
            const ScancodeBindings& scancodeBidings = input.getScancodeBindings();
            for (size_t i = 0; i < static_cast<size_t>(InputVerb::VerbCount); i++) {
                ImGui::Dummy(ImVec2{0.f, 25.f});
                std::string currentVerb = inputVerbToString(static_cast<InputVerb>(i)).c_str();
                ImGui::Text("%s: ", currentVerb.c_str());
                for (int j = 0; j < MaxBindsPerVerb; j++) {
                    std::string current = SDL_GetScancodeName(scancodeBidings[i][j].scancode);
                    current += "##" + currentVerb + "Index" + std::to_string(j);
                    ImGui::Button(current.c_str(), ImVec2{200.f, 50.f});
                    ImGui::SameLine();
                    ImGui::Dummy(ImVec2{10.f, 0.f});
                    ImGui::SameLine();
                }
                ImGui::NewLine();
            }
            ImGui::PopFont();
            ImGui::EndTabItem();
        }
        ImGui::PopFont();
    }
    ImGui::EndTabBar();
    ImGui::PopFont();
    ImGui::End();
}

void UiManager::drawPlayerSourceSetup(InputManager& input) {
    setNextWindowFullscreen();
    ImGui::Begin("Player Source Setup", nullptr, staticFlags);
    ImGui::PushFont(fontExtraLarge);
    ImGui::Text("Player Source Setup:");
    ImGui::PopFont();
    ImGui::PushFont(fontLarge);
    ImGui::Dummy(ImVec2{0.f, 10.f});
    ImGui::Text("Press any button to join!");
    ImGui::Dummy(ImVec2{0.f, 25.f});
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
            if (ImGui::Button(buttonId.c_str(), ImVec2{150.f, 50.f}) &&
                !playerSourceAddedThisFrame) {
                input.removePlayerSourceAtIndex(i);
            }
        }
        ImGui::Dummy(ImVec2{0.f, 50.f});
    }
    ImGui::PushFont(fontExtraLarge);
    if (ImGui::Button("Play", ImVec2{200.f, 60.f})) {
        const size_t playerSourceCount = input.getPlayerSourceCount();
        if (playerSourceCount > 0 && !playerSourceAddedThisFrame) {
            GameEvents::Push(GameEventTypes::ShouldDetectNewPlayerSources{false});
            GameEvents::Push(GameEventTypes::SetLevelName{LevelName::Test});
            // To make sure the ui screen is switched after the level is loaded.
            GameEvents::Push(GameEventTypes::SetUiState{UiState::Playing});
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Back", ImVec2{200.f, 60.f})) {
        runCancelEvent();
    }
    ImGui::PopFont();
    ImGui::PopFont();
    ImGui::End();
}

void UiManager::drawPauseMenu() {
    drawLargeLogo();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    ImVec2 menuPos = ImVec2{workPos.x + workSize.x * 0.5f, workPos.y + workSize.y * 0.5f};
    ImVec2 menuSize = ImVec2{425.f, 450.f};
    ImGui::SetNextWindowPos(menuPos, ImGuiCond_Always, ImVec2{0.5, 0.5});
    ImGui::SetNextWindowSize(menuSize);
    if (ImGui::Begin("Pause Menu", nullptr, staticFlags)) {
        ImGui::PushFont(fontLarge);
        float verticalSpacing = 15.f;
        float windowWidth = ImGui::GetWindowSize().x;
        float buttonWidth = 350.f;
        float buttonHeight = 75.f;
        float cursorX = (windowWidth - buttonWidth) * 0.5f;
        ImGui::SetCursorPosY(50.f);
        const char pausedText[] = "> Paused <";
        ImVec2 pauseTextSize = ImGui::CalcTextSize(pausedText);
        ImGui::SetCursorPosX((windowWidth - pauseTextSize.x) * 0.5f);
        ImGui::Text(pausedText);
        ImGui::Dummy(ImVec2(0, 50.f));
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