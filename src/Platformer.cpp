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
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -60.f};
    world = b2CreateWorld(&worldDef);
    SDL_Log("Created box2d world");
    camera.minViewableY = 0.f;
    UserInterface::keybindsUpdate(input);
    DiscordRpcManager::init();
    DiscordRpcManager::setStatus("In Development...", nullptr);
    // Test player
    b2BodyDef playerBodyDef = b2DefaultBodyDef();
    playerBodyDef.fixedRotation = false;
    b2ShapeDef playerShapeDef = b2DefaultShapeDef();
    playerShapeDef.density = 1.f;
    entities.push_back(
        std::make_unique<Entity>(
            world,
            b2MakeBox(0.5f, 0.5f),
            b2Vec2{0.f, 4.f},
            hexToColor(0xBA988AFF),
            false,
            playerBodyDef,
            playerShapeDef
        )
    );
    player = entities.front().get();
    camera.entityToFollow = player;
    entityController.setEntity(*player);
    entityController.spawnPoint = b2Vec2{0.f, 4.f};
    // Tempoaray test entities
    entities.push_back(
        std::make_unique<Entity>(
            world, b2MakeBox(50.f, 1.5f), b2Vec2{0.f, 0.f}, Colors.GrassGreen, true
        )
    );
    entities.push_back(
        std::make_unique<Entity>(
            world, b2MakeBox(0.5f, 10.f), b2Vec2{-18.f, 11.5f}, Colors.Gray, true
        )
    );
    entities.push_back(
        std::make_unique<Entity>(
            world, b2MakeBox(0.5f, 10.f), b2Vec2{18.f, 11.5f}, Colors.Gray, true
        )
    );
    b2BodyDef dynamicBodyDef = b2DefaultBodyDef();
    dynamicBodyDef.type = b2_dynamicBody;
    entities.push_back(
        std::make_unique<Entity>(
            world,
            b2MakeBox(0.5f, 2.f),
            b2Vec2{10.f, 3.f},
            Colors.Brown,
            false,
            dynamicBodyDef,
            b2DefaultShapeDef()
        )
    );
    entities.push_back(
        std::make_unique<Entity>(
            world,
            b2MakeBox(0.5f, 1.f),
            b2Vec2{-10.f, 3.f},
            Colors.Brown,
            false,
            dynamicBodyDef,
            b2DefaultShapeDef()
        )
    );
    // Temporary test tiles
    tiles.push_back(std::make_unique<Tile>(Vec2Int{0, 10}, assets, GameAssets::Textures::Test));
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
            if (player) {
                entityController.handleInput(*inputEvent, &camera);
            }
        }
    }
}

float Platformer::physicsStepHandler() {
    currentTime = SDL_GetTicks();
    float deltaTime = (float)(currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    }
    accumulator += deltaTime;
    while (accumulator >= physicsStep) {
        for (const auto& entity : entities) {
            if (entity) {
                entity->savePreviousState();
            }
        }
        entityController.update();
        b2World_Step(world, physicsStep, 4);
        accumulator -= physicsStep;
    }
    return accumulator / physicsStep;
}

void Platformer::run() {
    TTF_Text* text = assets.getText("Player", GameAssets::Fonts::Monocraft, 20.f); // Just to test
    audio.playMusic(GameAssets::Music::Test, 30, 1.f, true);
    running = true;
    while (running) {
        const Uint64 frameStartNs = SDL_GetTicksNS();
        handleSdlEvent();
        handleGameEvent();
        float alpha = physicsStepHandler();
        window.clearFrame();
        camera.run(alpha);
        for (const auto& entity : entities) {
            if (entity) {
                entity->draw(window, alpha);
                if (showFanTriangulation) {
                    b2Transform transform;
                    transform.p = entity->getInterpolatedPosition(alpha);
                    transform.q = entity->getInterpolatedRotation(alpha);
                    Drawing::showFanTriangulation(entity->getPolygon(), transform, window);
                }
            }
        }
        for (const auto& tile : tiles) {
            if (tile) {
                tile->draw(window);
            }
        }
        b2Vec2 textPos = player->getInterpolatedPosition(alpha);
        textPos.y += 2.f;
        Drawing::text(window, text, assets.textResolutionScaleFactor, textPos);
        UserInterface::keybindsShow();
        UserInterface::audio(audio);
        UserInterface::debug(window, player, entityController, camera, input, showFanTriangulation);
        window.render(frameStartNs);
        DiscordRpcManager::update();
    }
}

void Platformer::close() {
    DiscordRpcManager::shutdown();
    entities.clear();
    SDL_Log("Cleared entities");
    b2DestroyWorld(world);
    SDL_Log("Destroyed box2d world");
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
