#include "UiManager.hpp"
#include <cassert>

UiManager::UiManager(AssetManager& assets, UiState startingState) : currentState(startingState) {
    ImGuiIO& io = ImGui::GetIO();
    monocraftSmall = io.Fonts->AddFontFromFileTTF(
        assets.getFontPath(GameAssets::Fonts::Monocraft).c_str(), 12.f
    );
    monocraftMedium = io.Fonts->AddFontFromFileTTF(
        assets.getFontPath(GameAssets::Fonts::Monocraft).c_str(), 24.f
    );
    monocraftLarge = io.Fonts->AddFontFromFileTTF(
        assets.getFontPath(GameAssets::Fonts::Monocraft).c_str(), 36.f
    );
}

void UiManager::setState(UiState state) {
    assert(state >= static_cast<UiState>(0) && state < UiState::UiStateCount);
    currentState = state;
}

UiState UiManager::getState() const {
    return currentState;
}

void UiManager::render(
    WindowManager& window,
    AudioManager& audio,
    InputManager& input,
    Level* level,
    bool& showFanTriangulation
) {
    Entity* player = nullptr;
    Camera* camera = nullptr;
    if (level) {
        const auto& players = level->getPlayers();
        if (players.size() > 0) {
            player = players[0]->getEntity();
        }
        camera = &level->camera;
    }
    switch (currentState) {
    case UiState::MainMenu: {
        drawMainMenu();
    }; break;
    case UiState::Settings: {
        drawSettings(window, showFanTriangulation, audio);
    }; break;
    case UiState::Paused: {
        drawPauseMenu();
    }; break;
    case UiState::PausedSettings: {
        drawSettings(window, showFanTriangulation, audio);
    }; break;
    default:
        break;
    }
    if (showDebug) {
        drawDebug(window, player, camera, input);
    }
}

void UiManager::runCancelEvent() {
    switch (currentState) {
    case UiState::Settings:
        currentState = UiState::MainMenu;
        break;
    case UiState::Playing:
        currentState = UiState::Paused;
        break;
    case UiState::Paused: {
        currentState = UiState::Playing;
        break;
    case UiState::PausedSettings:
        currentState = UiState::Paused;
        break;
    default:
        break;
    }
    }
}

void UiManager::drawDebug(
    WindowManager& window, Entity* player, Camera* camera, InputManager& input
) {
    ImGui::Begin("Debug Menu");
    ImGui::PushFont(monocraftSmall);
    ImGui::Text("Window:");
    WindowDimensions windowSize = window.getSize();
    ImGui::Text(
        "Size: %d, %d\n%.1f/%s FPS (%.3f ms/frame)",
        windowSize.x,
        windowSize.y,
        ImGui::GetIO().Framerate,
        window.targetFpsStr().c_str(),
        1000.0f / ImGui::GetIO().Framerate
    );
    ImGui::Dummy(ImVec2{1.f, 1.f});
    if (player) {
        ImGui::Dummy(ImVec2{1.f, 1.f});
        b2Vec2 position = b2Body_GetPosition(player->getBodyId());
        b2Vec2 velocity = b2Body_GetLinearVelocity(player->getBodyId());
        ImGui::Text("\nPlayer:");
        ImGui::Text(
            "Position: %.2f, %.2f\nVelocity: %.2f, %.2f",
            position.x,
            position.y,
            velocity.x,
            velocity.y
        );
    }
    if (camera && camera->entityToFollow) {
        ImGui::Dummy(ImVec2{1.f, 1.f});
        b2Vec2 safeAreaSize = camera->getSafeAreaSize();
        b2Vec2 safeAreaValue = camera->getEntitySafeAreaValue();
        ImGui::Text("\nSafe Area:");
        ImGui::Text(
            "Size: %.2f, %.2f\nRatio from Center: %.2f, %.2f",
            safeAreaSize.x,
            safeAreaSize.y,
            safeAreaValue.x,
            safeAreaValue.y
        );
    }
    ImGui::Dummy(ImVec2{1.f, 1.f});
    size_t controllersConnected = input.getGamepadCount();
    ImGui::Text("\nControllers Connected: %zu", controllersConnected);
    ImGui::PopFont();
    ImGui::End();
}

void UiManager::drawMainMenu() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    ImVec2 menuPos = ImVec2{workPos.x + workSize.x * 0.5f, workPos.y + workSize.y * 0.5f};
    ImVec2 menuSize = ImVec2{425.f, 375.f};
    ImGui::SetNextWindowPos(menuPos, ImGuiCond_Always, ImVec2{0.5, 0.5});
    ImGui::SetNextWindowSize(menuSize);
    if (ImGui::Begin("Main Menu", nullptr, staticFlags)) {
        ImGui::PushFont(monocraftLarge);
        float verticalSpacing = 15.f;
        float windowWidth = ImGui::GetWindowSize().x;
        float buttonWidth = 350.f;
        float buttonHeight = 75.f;
        float cursorX = (windowWidth - buttonWidth) * 0.5f;
        ImGui::SetCursorPosY(50.f);
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Test Game", ImVec2{buttonWidth, buttonHeight})) {
            GameEvents::Push(GameEventTypes::SetLevelName{LevelName::Template});
            currentState = UiState::Playing;
        }
        ImGui::Dummy(ImVec2(0, verticalSpacing));
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Settings", ImVec2{buttonWidth, buttonHeight})) {
            currentState = UiState::Settings;
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
    WindowManager& window, bool& showFanTriangulation, AudioManager& audio
) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    ImVec2 menuPos = ImVec2{workPos.x + workSize.x * 0.5f, workPos.y + workSize.y * 0.5f};
    ImGui::SetNextWindowPos(menuPos, ImGuiCond_Always, ImVec2{0.5, 0.5});
    ImGui::SetNextWindowSize(workSize);
    const ImVec2 verticalSpacingDummy{0.f, 10.f};
    const ImVec2 horizontalSpacingDummy{10.f, 0.f};
    if (ImGui::Begin("Settings", nullptr, staticFlags)) {
        ImGui::PushFont(monocraftMedium);
        if (ImGui::Button("Back", ImVec2{60.f, 30.f})) {
            runCancelEvent();
        }
        ImGui::SameLine();
        ImGui::Dummy(horizontalSpacingDummy);
        ImGui::SameLine();
        ImGui::BeginTabBar("SettingsTabBar");
        if (ImGui::BeginTabItem("Display")) {
            ImGui::PushFont(monocraftLarge);
            ImGui::Text("Display");
            ImGui::PopFont();
            ImGui::Dummy(verticalSpacingDummy);
            ImGui::Text(
                "%.0f/%s FPS (%.3f ms/frame)",
                ImGui::GetIO().Framerate,
                window.targetFpsStr().c_str(),
                1000.0f / ImGui::GetIO().Framerate
            );
            ImGui::Dummy(verticalSpacingDummy);
            bool vsync = window.isVsyncEnabled();
            if (ImGui::Checkbox("Vsync (Idk if this is working)", &vsync)) {
                window.setVsync(vsync);
            }
            ImGui::Dummy(verticalSpacingDummy);
            bool fpsUnlimited = window.getFpsUnlimited();
            if (ImGui::Checkbox("FPS Unlimited", &fpsUnlimited)) {
                window.setFpsUnlimited(fpsUnlimited);
            }
            ImGui::Dummy(verticalSpacingDummy);
            if (!vsync && !fpsUnlimited) {
                static int tempFps = window.getTargetFps();
                ImGui::SliderInt("Target FPS", &tempFps, 10, 300);
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    window.setTargetFps(tempFps);
                }
            }
            ImGui::Dummy(verticalSpacingDummy);
            ImGui::Checkbox("Show Triangles", &showFanTriangulation);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Audio")) {
            ImGui::PushFont(monocraftLarge);
            ImGui::Text("Audio");
            ImGui::PopFont();
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
            if (ImGui::SliderInt("Master", &masterVolume, 0, MaxVolume)) {
                audio.setVolume(AudioCategory::Master, masterVolume);
            }
            ImGui::Dummy(verticalSpacingDummy);
            if (ImGui::Button("Reset##ResetSounds", resetButtonSize)) {
                audio.setVolume(AudioCategory::Sounds, 100);
            }
            ImGui::SameLine();
            ImGui::Dummy(horizontalSpacingDummy);
            ImGui::SameLine();
            if (ImGui::SliderInt("Sounds", &soundVolume, 0, MaxVolume)) {
                audio.setVolume(AudioCategory::Sounds, soundVolume);
            }
            ImGui::Dummy(verticalSpacingDummy);
            if (ImGui::Button("Reset##ResetMusic", resetButtonSize)) {
                audio.setVolume(AudioCategory::Music, 100);
            }
            ImGui::SameLine();
            ImGui::Dummy(horizontalSpacingDummy);
            ImGui::SameLine();
            if (ImGui::SliderInt("Music", &musicVolume, 0, MaxVolume)) {
                audio.setVolume(AudioCategory::Music, musicVolume);
            }
            ImGui::Dummy(verticalSpacingDummy);
            if (ImGui::Button("Reset##ResetMusicPitch", resetButtonSize)) {
                audio.setMusicPitch(1.f);
            }
            ImGui::SameLine();
            ImGui::Dummy(horizontalSpacingDummy);
            ImGui::SameLine();
            if (ImGui::SliderFloat("Music Pitch", &pitch, 0.25f, 2.f, "%.2f")) {
                audio.setMusicPitch(pitch);
            }
            ImGui::Dummy(verticalSpacingDummy);
            ImGui::Text(
                "Current Music: %s %s",
                audio.getCurrentMusicName(),
                audio.isMusicLooping() ? "(Looping)" : ""
            );
            ImGui::Text("Timestamp: %s", audio.formattedMusicTime().c_str());
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Controls")) {
            ImGui::PushFont(monocraftLarge);
            ImGui::Text("Controls");
            ImGui::PopFont();
            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();
    ImGui::PopFont();
    ImGui::End();
}

void UiManager::drawPauseMenu() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    ImVec2 menuPos = ImVec2{workPos.x + workSize.x * 0.5f, workPos.y + workSize.y * 0.5f};
    ImVec2 menuSize = ImVec2{425.f, 450.f};
    ImGui::SetNextWindowPos(menuPos, ImGuiCond_Always, ImVec2{0.5, 0.5});
    ImGui::SetNextWindowSize(menuSize);
    if (ImGui::Begin("Pause Menu", nullptr, staticFlags)) {
        ImGui::PushFont(monocraftLarge);
        float verticalSpacing = 15.f;
        float windowWidth = ImGui::GetWindowSize().x;
        float buttonWidth = 350.f;
        float buttonHeight = 75.f;
        float cursorX = (windowWidth - buttonWidth) * 0.5f;
        ImGui::SetCursorPosY(50.f);
        const char* pausedText = "> Paused <";
        ImVec2 pauseTextSize = ImGui::CalcTextSize(pausedText);
        ImGui::SetCursorPosX((windowWidth - pauseTextSize.x) * 0.5f);
        ImGui::Text("> Paused <");
        ImGui::Dummy(ImVec2(0, 50.f));
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Resume", ImVec2{buttonWidth, buttonHeight})) {
            currentState = UiState::Playing;
        }
        ImGui::Dummy(ImVec2(0, verticalSpacing));
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Settings", ImVec2{buttonWidth, buttonHeight})) {
            currentState = UiState::PausedSettings;
        }
        ImGui::Dummy(ImVec2(0, verticalSpacing));
        ImGui::SetCursorPosX(cursorX);
        if (ImGui::Button("Exit", ImVec2{buttonWidth, buttonHeight})) {
            GameEvents::Push(GameEventTypes::SetLevelName{LevelName::None});
            currentState = UiState::MainMenu;
        }
        ImGui::PopFont();
    }
    ImGui::End();
}