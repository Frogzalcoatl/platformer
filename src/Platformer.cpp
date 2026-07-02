#include "Platformer.hpp"
#include "Colors.hpp"
#include "DiscordRpcManager.hpp"
#include "Drawing.hpp"
#include "Events.hpp"
#include "UserInterface/Index.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

Platformer::Platformer()
    : window{"C++ Platformer", Colors.BackGround}, assets{window.getSdlRenderer()}, audio{assets},
      camera{Camera{nullptr, window}} {
    UserInterface::keybindsUpdate(input);
    DiscordRpcManager::init();
    DiscordRpcManager::setStatus("In Development...", nullptr);
    currentLevel = getTemplateLevel(assets);
    const auto& players = currentLevel->getPlayers();
    if (players.size() >= 1) {
        camera.entityToFollow = players[0]->getEntity();
    }
    camera.minViewableY = 0.f;
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
        case SDL_EVENT_GAMEPAD_REMOVED:
            input.handleGamepadDeviceEvent(event.gdevice);
        };
        break;
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
        } else if (const auto* inputEvent = std::get_if<GameEventTypes::Input>(&event)) {
            if (inputEvent->state == InputState::Pressed) {
                switch (inputEvent->verb) {
                case InputVerb::ToggleFullscreen:
                    window.toggleFullscreen();
                    break;
                case InputVerb::ZoomIn:
                    window.incrementScaleMultiplierBy(0.05f);
                    break;
                case InputVerb::ZoomOut:
                    window.incrementScaleMultiplierBy(-0.05f);
                    break;
                case InputVerb::ZoomReset:
                    window.resetScaleMultiplier();
                default:
                    break;
                }
            }
            currentLevel->handleInput(*inputEvent, &camera);
        }
    }
}

void Platformer::run() {
    TTF_Text* text = assets.getText("Player", GameAssets::Fonts::Monocraft, 20.f); // Just to test
    audio.playMusic(GameAssets::Music::Test, 30, 1.f, true);
    running = true;
    while (running) {
        const Uint64 frameStartNs = SDL_GetTicksNS();
        handleSdlEvent();
        handleGameEvent();
        float alpha = currentLevel->update();
        window.clearFrame();
        camera.run(alpha);
        if (currentLevel) {
            currentLevel->draw(window, alpha, showFanTriangulation);
        }
        if (camera.entityToFollow) {
            b2Vec2 textPos = camera.entityToFollow->getInterpolatedPosition(alpha);
            textPos.y += 2.f;
            Drawing::text(window, text, assets.textResolutionScaleFactor, textPos);
        }
        UserInterface::keybindsShow();
        UserInterface::audio(audio);
        UserInterface::debug(window, camera.entityToFollow, camera, input, showFanTriangulation);
        window.render(frameStartNs);
        DiscordRpcManager::update();
    }
}

void Platformer::close() {
    DiscordRpcManager::shutdown();
    assets.closeAll();
    window.cleanup();
    ImGui_ImplSDLRenderer3_Shutdown();
    SDL_Log("Shutdown ImGui SDL3 renderer implementation");
    ImGui_ImplSDL3_Shutdown();
    SDL_Log("Shutdown ImGui SDL3 implementation");
    ImGui::DestroyContext();
    SDL_Log("Destroyed ImGui context");
    TTF_Quit();
    SDL_Log("Quit SDL_ttf");
    SDL_Quit();
    SDL_Log("Quit SDL");
}
