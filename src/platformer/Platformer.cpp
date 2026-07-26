#include "platformer/Platformer.hpp"
#include "system/DiscordRpcManager.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <stdexcept>

#ifdef SDL_PLATFORM_ANDROID
#include "system/Android.hpp"
#endif

Platformer::Platformer()
    : window{"C++ Platformer", Colors::Background}, assets(window.getSdlRenderer()), audio(assets),
      settings("Settings.json"), ui(assets) {
    loadSettings();
    assets.addGameControllerMappings("gamepads/gamecontrollerdb.txt");
    assets.addGameControllerMappings("gamepads/retrolink.txt");
}

void Platformer::loadSettings() {
    const Settings& currentSettings = settings.get();
#ifdef SDL_PLATFORM_ANDROID
    // Cannot toggle vsync on android anyways
    window.setVsync(false);
#else
    window.setVsync(currentSettings.vsyncEnabled);
#endif
    const unsigned int MinTargetFps = 1;
    const unsigned int MaxTargetFps = 1000;
    if (currentSettings.targetFps < MinTargetFps) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Clamping target fps from %u to %u",
            currentSettings.targetFps,
            MinTargetFps
        );
        settings.setTargetFps(MinTargetFps);
    } else if (currentSettings.targetFps > MaxTargetFps) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Clamping target fps from %u to %u",
            currentSettings.targetFps,
            MaxTargetFps
        );
        settings.setTargetFps(MaxTargetFps);
    }
    if (settings.createdNewFileOnRead()) {
        const Uint64 monitorRefreshRate =
            static_cast<Uint64>(SDL_roundf(window.getMonitorRefreshRate()));
        settings.setTargetFps(static_cast<unsigned int>(monitorRefreshRate));
        window.setTargetFps(monitorRefreshRate);
    } else {
        window.setTargetFps(currentSettings.targetFps);
    }
    window.setFpsUnlimited(currentSettings.fpsUnlimited);
    ui.setScaleIndex(currentSettings.uiScale);
    size_t userPreferredScale = ui.getScaleIndex();
    if (userPreferredScale != currentSettings.uiScale) {
        settings.setUiScale(userPreferredScale);
    }
    const unsigned int MaxVolume = 200;
    if (currentSettings.masterVolume > MaxVolume) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Clamping master volume from %u to %u",
            currentSettings.masterVolume,
            MaxVolume
        );
        settings.setMasterVolume(MaxVolume);
    }
    if (currentSettings.soundsVolume > MaxVolume) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Clamping sounds volume from %u to %u",
            currentSettings.soundsVolume,
            MaxVolume
        );
        settings.setSoundsVolume(MaxVolume);
    }
    if (currentSettings.musicVolume > MaxVolume) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Clamping music volume from %u to %u",
            currentSettings.musicVolume,
            MaxVolume
        );
        settings.setMusicVolume(MaxVolume);
    }
    audio.setVolume(AudioCategory::Master, currentSettings.masterVolume);
    audio.setVolume(AudioCategory::Sounds, currentSettings.soundsVolume);
    audio.setVolume(AudioCategory::Music, currentSettings.musicVolume);
}

void Platformer::handleSdlEvent() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        switch (event.type) {
        case SDL_EVENT_QUIT: {
            running = false;
#ifdef SDL_PLATFORM_ANDROID
            Android::quitAndRemoveTask();
#endif
        }; break;
        case SDL_EVENT_WINDOW_RESIZED: {
            window.handleResize(event.window.data1, event.window.data2);
            if (currentLevel) {
                Camera* camera = currentLevel->getCamera();
                if (camera) {
                    camera->handleWindowResize(event.window.data1, event.window.data2);
                }
            }
        }; break;
        case SDL_EVENT_MOUSE_MOTION: {
            window.handleMouseMotionEvent(event.motion);
        }; break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_MOUSE_WHEEL:
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            std::vector<GameEventTypes::Input> inputEvents =
                input.getInputEventsFromSDLEvent(event);
            for (const auto& inputEvent : inputEvents) {
                GameEvents::Push(inputEvent);
            }
        }; break;
        case SDL_EVENT_PINCH_BEGIN:
        case SDL_EVENT_PINCH_UPDATE:
        case SDL_EVENT_PINCH_END: {
            if (ui.getFreeFingerCount() >= 2) {
                input.handlePinchEvent(event.pinch);
            }
        }; break;
        case SDL_EVENT_GAMEPAD_ADDED: {
            GameEvents::Schedule(
                GameEventTypes::GamepadConnectedNotification{event.gdevice.which}, 25
            );
        }; break;
        case SDL_EVENT_GAMEPAD_REMOVED: {
            input.handleGamepadRemoved(event.gdevice);
            std::string message =
                "Controller Disconnected: " + input.getGamepadName(event.gdevice.which);
            notificationManager.send(message);
        }; break;
        case SDL_EVENT_DID_ENTER_BACKGROUND: {
            settings.saveToDisk();
        }; break;
        };
    }
}

void Platformer::handleInputGameEvent(const GameEventTypes::Input& inputEvent) {
    UiState uiState = ui.getState();
    if (inputEvent.state == InputState::Pressed) {
        switch (inputEvent.verb) {
        case InputVerb::ToggleFullscreen:
#if defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_MACOS) || defined(SDL_PLATFORM_LINUX)
            window.toggleFullscreen(); // Toggling fullscreen should only be accessible on desktop
#endif
            break;
        case InputVerb::ZoomIn:
            if (currentLevel && uiState == UiState::Playing) {
                Camera* camera = currentLevel->getCamera();
                if (camera) {
                    camera->incrementScaleMultiplierBy(0.05f);
                }
            }
            break;
        case InputVerb::ZoomOut:
            if (currentLevel && uiState == UiState::Playing) {
                Camera* camera = currentLevel->getCamera();
                if (camera) {
                    camera->incrementScaleMultiplierBy(-0.05f);
                }
            }
            break;
        case InputVerb::ZoomReset:
            if (currentLevel && uiState == UiState::Playing) {
                Camera* camera = currentLevel->getCamera();
                if (camera) {
                    camera->resetScaleMultiplier();
                }
            }
            break;
        case InputVerb::ToggleDebug:
            ui.toggleDebug();
            break;
        case InputVerb::Cancel:
            // Purposely continuing into pause, cancel and pause are nearly identical
            // Only difference is cancel cannot be used to pause the game
            if (uiState == UiState::Playing) {
                break;
            }
            SDL_FALLTHROUGH;
        case InputVerb::Pause:
            if (ImGui::IsAnyItemActive()) {
                break;
            }
            ui.runCancelEvent();
            if (currentLevel) {
                const auto& players = currentLevel.get()->getPlayers();
                for (const auto& player : players) {
                    if (player.controller) {
                        player.controller->resetInput();
                    }
                }
            }
            break;
        case InputVerb::ShowHitboxes:
            if (currentLevel) {
                currentLevel->showHitBoxes = !currentLevel->showHitBoxes;
            }
        default:
            break;
        }
    }
    if (currentLevel && uiState == UiState::Playing) {
        currentLevel->handleInput(inputEvent);
    }
    ui.passInputToImGui(inputEvent);
}

void Platformer::handleGameEvent() {
    GameEvents::UpdateScheduledEvents();
    GameEvent event;
    while (GameEvents::Poll(event)) {
        if (std::holds_alternative<GameEventTypes::CloseWindow>(event)) {
            running = false;
#ifdef SDL_PLATFORM_ANDROID
            Android::quitAndRemoveTask();
#endif
        } else if (const auto* playSoundEvent = std::get_if<GameEventTypes::PlaySound>(&event)) {
            audio.playSound(
                playSoundEvent->relativePath, playSoundEvent->volume, playSoundEvent->pitch
            );
        } else if (const auto* playMusicEvent = std::get_if<GameEventTypes::PlayMusic>(&event)) {
            audio.playMusic(
                playMusicEvent->relativePath,
                playMusicEvent->volume,
                playMusicEvent->pitch,
                playMusicEvent->loop
            );
        } else if (const auto* setVolume = std::get_if<GameEventTypes::SetVolume>(&event)) {
            audio.setVolume(setVolume->category, setVolume->volume);
        } else if (const auto* inputEvent = std::get_if<GameEventTypes::Input>(&event)) {
            handleInputGameEvent(*inputEvent);
        } else if (const auto* setUiState = std::get_if<GameEventTypes::SetUiState>(&event)) {
            ui.setState(setUiState->state);
        } else if (const auto* setLevelName = std::get_if<GameEventTypes::SetLevelName>(&event)) {
            const LevelAssetsVector& previousLevelAssets =
                currentLevel ? currentLevel->getRequiredAssets() : LevelAssetsVector{};
            if (setLevelName->level == LevelName::None) {
                currentLevel->unloadRequiredAssets(assets);
                currentLevel = nullptr;
                window.backgroundColor = Colors::Background;
                continue;
            } else if (setLevelName->level == LevelName::Test) {
                currentLevel = getTestLevel(assets, window, audio, previousLevelAssets);
            }
            currentLevel->updatePlayers(input.getPlayerSources(), assets);
            size_t touchPlayerIndex;
            if (!input.isTouchPlayerEnabled(&touchPlayerIndex)) {
                ui.disableTouchController();
            } else {
                Entity* touchEntity = currentLevel->getPlayerEntity(touchPlayerIndex);
                if (touchEntity) {
                    ui.enableTouchController(*touchEntity);
                } else {
                    ui.disableTouchController();
                }
            }
            window.backgroundColor = currentLevel->backgroundColor;
        } else if (
            const auto* playerSourceAdded = std::get_if<GameEventTypes::PlayerSourceAdded>(&event)
        ) {
            (void)playerSourceAdded;
            if (currentLevel) {
                currentLevel->updatePlayers(input.getPlayerSources(), assets);
            }
            ui.setPlayerSourceAddedThisFrame(true);
            if (ui.getState() != UiState::PlayerSourceSetup) {
                std::string notification = "Player Source Added: \"" +
                                           input.getSourceName(playerSourceAdded->source) + "\"";
                notificationManager.send(notification);
            }
        } else if (
            const auto* playerSourceRemoved =
                std::get_if<GameEventTypes::PlayerSourceRemoved>(&event)
        ) {
            if (currentLevel) {
                currentLevel->updatePlayers(input.getPlayerSources(), assets);
            }
            if (ui.getState() != UiState::PlayerSourceSetup) {
                std::string notification = "Player Source Removed: \"" +
                                           input.getSourceName(playerSourceRemoved->source) + "\"";
                notificationManager.send(notification);
            }
        } else if (
            const auto* detectNewPlayers =
                std::get_if<GameEventTypes::ShouldDetectNewPlayerSources>(&event)
        ) {
            if (detectNewPlayers->value) {
                input.listenForValidKeyboard = true;
                input.listenForNewGamepad = true;
                input.enableTouchPlayer(); // Is ignored if user does not have touch screen
                SDL_Log("Enabled input detection for adding new player sources.");
            } else {
                bool valueWillBeChanged = input.listenForValidKeyboard || input.listenForNewGamepad;
                input.listenForValidKeyboard = false;
                input.listenForNewGamepad = false;
                if (valueWillBeChanged) {
                    SDL_Log("Disabled input detection for adding new player sources.");
                }
            }
        } else if (
            const auto* changeLevelZoom = std::get_if<GameEventTypes::ChangeLevelZoom>(&event)
        ) {
            if (currentLevel && ui.getState() == UiState::Playing) {
                Camera* camera = currentLevel->getCamera();
                if (camera) {
                    camera->incrementScaleMultiplierBy(changeLevelZoom->amount);
                }
            }
        } else if (
            const auto* sendNotification = std::get_if<GameEventTypes::SendNotification>(&event)
        ) {
            notificationManager.send(sendNotification->message, sendNotification->onClick);
        } else if (
            const auto* gamepadEventNotification =
                std::get_if<GameEventTypes::GamepadConnectedNotification>(&event)
        ) {
            std::string message =
                "Controller Connected: " + input.getGamepadName(gamepadEventNotification->id);
            notificationManager.send(message);
        } else if (const auto* saveUserData = std::get_if<GameEventTypes::SaveUserData>(&event)) {
            if (saveUserData->type == UserDataTypes::Settings) {
                settings.saveToDisk();
            }
        }
    }
}

// Just for testing
static bool playMusicFailed = false;
const std::vector<const char*> MusicFileNames = {
    "2023_4 (unfinished).ogg",
    "2023_11(3).ogg",
    "2023_14.ogg",
    "2023_23.ogg",
    "2023_29.ogg",
    "2023_35.ogg",
    "2023_37.ogg",
    "2024_3.ogg",
    "2024_5.ogg",
    "2024_7.ogg",
    "2024_8.ogg",
    "2026_2.ogg",
    "2026_3.ogg",
    "2026_4.ogg",
    "2026_5.ogg",
    "2026_6.ogg"
};

void Platformer::run() {
    running = true;
    while (running) {
        if (!audio.isMusicPlaying() && !playMusicFailed) {
            size_t randomSongId = static_cast<size_t>(
                SDL_floorf(static_cast<float>(MusicFileNames.size()) * SDL_randf())
            );
            std::string relativePath = "music/";
            relativePath += MusicFileNames[randomSongId];
            if (!audio.playMusic(relativePath, 100, audio.getMusicPitch(), false)) {
                playMusicFailed = true;
            }
        }
        const Uint64 frameStartNs = SDL_GetTicksNS();
        handleSdlEvent();
        handleGameEvent();
        window.clearFrame();
        if (currentLevel) {
            UiState currentState = ui.getState();
            if (currentState == UiState::Playing) {
                currentLevel->update();
            }
            currentLevel->draw(window, assets);
        }
        ui.update(window, settings, audio, input, currentLevel.get());
        notificationManager.update(window, ui.getActualScale());
        window.render(frameStartNs);
        DiscordRpcManager::update();
    }
    settings.saveToDisk();
}
