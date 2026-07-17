#include "Platformer.hpp"
#ifdef SDL_PLATFORM_ANDROID
#include "Android.hpp"
#endif
#include "Colors.hpp"
#include "DiscordRpcManager.hpp"
#include "Drawing.hpp"
#include "Events.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <stdexcept>

Platformer::Platformer()
    : window{"C++ Platformer", Colors::Background}, assets{window.getSdlRenderer()}, audio{assets},
      ui{assets} {
    audio.setVolume(AudioCategory::Music, 50);
    assets.addGameControllerMappings("gamepads/gamecontrollerdb.txt");
    assets.addGameControllerMappings("gamepads/retrolink.txt");
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
        case SDL_EVENT_GAMEPAD_REMOVED: {
            input.handleGamepadRemoved(event.gdevice);
        }; break;
        };
    }
}

void Platformer::handleGameEventInput(const GameEventTypes::Input& inputEvent) {
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
            ui.showDebug = !ui.showDebug;
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
            handleGameEventInput(*inputEvent);
        } else if (const auto* setUiState = std::get_if<GameEventTypes::SetUiState>(&event)) {
            ui.setState(setUiState->state);
        } else if (const auto* setLevelName = std::get_if<GameEventTypes::SetLevelName>(&event)) {
            if (setLevelName->level == LevelName::None) {
                currentLevel = nullptr;
                window.backgroundColor = Colors::Background;
                continue;
            } else if (setLevelName->level == LevelName::Test) {
                currentLevel = getTestLevel(assets, window);
            }
            currentLevel->updatePlayers(input.getPlayerSources(), assets);
            window.backgroundColor = currentLevel->backgroundColor;
        } else if (
            const auto* playerSourceAdded = std::get_if<GameEventTypes::PlayerSourceAdded>(&event)
        ) {
            // TODO: Some sort of ui stating a player has been added
            (void)playerSourceAdded;
            if (currentLevel) {
                currentLevel->updatePlayers(input.getPlayerSources(), assets);
            }
            ui.setPlayerSourceAddedThisFrame(true);
        } else if (
            const auto* playerSourceRemoved =
                std::get_if<GameEventTypes::PlayerSourceRemoved>(&event)
        ) {
            // TODO: some sort of ui stating a player has been disconnected.
            // Maybe a reconnect input source prompt
            (void)playerSourceRemoved;
            if (currentLevel) {
                currentLevel->updatePlayers(input.getPlayerSources(), assets);
            }
        } else if (
            const auto* detectNewPlayers =
                std::get_if<GameEventTypes::ShouldDetectNewPlayerSources>(&event)
        ) {
            if (detectNewPlayers->value) {
                SDL_Log("Enabled input detection for adding new player sources.");
                input.listenForValidKeyboard = true;
                input.listenForNewGamepad = true;
                input.enableTouchPlayer();
            } else {
                SDL_Log("Disabled input detection for adding new player sources.");
                input.listenForValidKeyboard = false;
                input.listenForNewGamepad = false;
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
            size_t randomSongId =
                static_cast<size_t>(SDL_floorf(MusicFileNames.size() * SDL_randf()));
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
        ui.update();
        ui.draw(window, audio, input, currentLevel.get(), ui);
        window.render(frameStartNs);
        DiscordRpcManager::update();
    }
}
