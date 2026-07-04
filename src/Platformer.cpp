#include "Platformer.hpp"
#include "Colors.hpp"
#include "DiscordRpcManager.hpp"
#include "Drawing.hpp"
#include "Events.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <stdexcept>

Platformer::Platformer()
    : window{"C++ Platformer", Colors.Background}, assets{window.getSdlRenderer()}, audio{assets},
      ui{assets} {
    audio.setVolume(AudioCategory::Music, 50);
    assets.initSDLGameControllerDB();
}

void Platformer::handleSdlEvent() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        switch (event.type) {
        case SDL_EVENT_QUIT: {
            running = false;
        }; break;
        case SDL_EVENT_WINDOW_RESIZED: {
            window.handleResize(event.window.data1, event.window.data2);
            if (currentLevel) {
                currentLevel->camera.handleWindowResize(event.window.data1, event.window.data2);
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
            auto inputEvents = input.getInputEventsFromSDLEvent(event);
            for (const auto& inputEvent : inputEvents) {
                GameEvents::Push(inputEvent);
            }
        }; break;
        case SDL_EVENT_GAMEPAD_ADDED:
        case SDL_EVENT_GAMEPAD_REMOVED: {
            input.handleGamepadDeviceEvent(event.gdevice);
        }; break;
        };
    }
}

void Platformer::handleGameEvent() {
    GameEvent event;
    while (GameEvents::Poll(event)) {
        if (std::holds_alternative<GameEventTypes::CloseWindow>(event)) {
            running = false;
        } else if (const auto* playSoundEvent = std::get_if<GameEventTypes::PlaySound>(&event)) {
            audio.playSound(playSoundEvent->soundId, playSoundEvent->volume, playSoundEvent->pitch);
        } else if (const auto* playMusicEvent = std::get_if<GameEventTypes::PlayMusic>(&event)) {
            audio.playMusic(
                playMusicEvent->musicId,
                playMusicEvent->volume,
                playMusicEvent->pitch,
                playMusicEvent->loop
            );
        } else if (const auto* setVolume = std::get_if<GameEventTypes::SetVolume>(&event)) {
            audio.setVolume(setVolume->category, setVolume->volume);
        } else if (const auto* inputEvent = std::get_if<GameEventTypes::Input>(&event)) {
            UiState uiState = ui.getState();
            if (uiState != UiState::Playing) {
                ui.passInputToImGui(*inputEvent);
            }
            if (inputEvent->state == InputState::Pressed) {
                switch (inputEvent->verb) {
                case InputVerb::ToggleFullscreen:
                    window.toggleFullscreen();
                    break;
                case InputVerb::ZoomIn:
                    if (currentLevel && uiState == UiState::Playing) {
                        currentLevel->camera.incrementScaleMultiplierBy(0.05f);
                    }
                    break;
                case InputVerb::ZoomOut:
                    if (currentLevel && uiState == UiState::Playing) {
                        currentLevel->camera.incrementScaleMultiplierBy(-0.05f);
                    }
                    break;
                case InputVerb::ZoomReset:
                    if (currentLevel && uiState == UiState::Playing) {
                        currentLevel->camera.resetScaleMultiplier();
                    }
                    break;
                case InputVerb::ToggleDebug:
                    ui.showDebug = !ui.showDebug;
                    break;
                case InputVerb::Cancel:
                    ui.runCancelEvent();
                    if (currentLevel) {
                        const auto& players = currentLevel.get()->getPlayers();
                        for (const auto& player : players) {
                            player->resetMovement();
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
                currentLevel->handleInput(*inputEvent);
            }
        } else if (const auto* setUiState = std::get_if<GameEventTypes::SetUiState>(&event)) {
            ui.setState(setUiState->state);
        } else if (const auto* setLevelName = std::get_if<GameEventTypes::SetLevelName>(&event)) {
            if (setLevelName->level == LevelName::Template) {
                currentLevel = getTemplateLevel(assets, window);
                window.backgroundColor = Colors.SkyBlue;
            } else if (setLevelName->level == LevelName::None) {
                currentLevel = nullptr;
                window.backgroundColor = Colors.Background;
            }
        }
    }
}

static bool playMusicFailed = false; // Just for testing

void Platformer::run() {
    running = true;
    UniqueText text =
        assets.getText("Player", GameAssets::Fonts::Monocraft, 20.f); // Just for testing
    while (running) {
        if (!audio.isMusicPlaying() && !playMusicFailed) {
            GameAssets::Music randomSong = static_cast<GameAssets::Music>(
                SDL_floorf(static_cast<float>(GameAssets::Music::MusicCount) * SDL_randf())
            );
            if (!audio.playMusic(randomSong, 100, audio.getMusicPitch(), false)) {
                playMusicFailed = true;
            }
        }
        const Uint64 frameStartNs = SDL_GetTicksNS();
        handleSdlEvent();
        handleGameEvent();
        window.clearFrame();
        if (currentLevel) {
            UiState currentState = ui.getState();
            float alpha = 0.f;
            if (currentState == UiState::Playing) {
                alpha = currentLevel->update();
            }
            currentLevel->draw(window, alpha);
            if (currentLevel->camera.entityToFollow) {
                b2Vec2 textPos =
                    currentLevel->camera.entityToFollow->getInterpolatedPosition(alpha);
                textPos.y += 2.f;
                Drawing::text(
                    text.get(),
                    window,
                    textPos,
                    currentLevel->camera.getScaleFactor(),
                    currentLevel->camera.getOffsetPixels(),
                    assets.TextResolutionScaleFactor
                );
            }
        }
        ui.render(window, audio, input, currentLevel.get());
        window.render(frameStartNs);
        DiscordRpcManager::update();
    }
}
