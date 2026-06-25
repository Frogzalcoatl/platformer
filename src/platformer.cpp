#include "platformer.hpp"
#include "colors.hpp"
#include "events.hpp"
#include "playerInput.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

Platformer::Platformer(void)
    : camera(Camera(nullptr, window)), window{"C++ Platformer", Colors.BackGround} {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -22.0f};
    world = b2CreateWorld(&worldDef);
    running = false;
    connectDefaultVerbMappings();
    camera.safeArea = b2Vec2{0.25f, 0.25f};
    camera.minViewableY = 0.f;
    // Test player
    b2BodyDef playerBodyDef = b2DefaultBodyDef();
    playerBodyDef.linearDamping = 0.5f;
    playerBodyDef.fixedRotation = true;
    b2ShapeDef playerShapeDef = b2DefaultShapeDef();
    playerShapeDef.density = 1.f;
    playerShapeDef.material.friction = 0.3f;
    player = new Entity(
        world, {0.5f, 0.5f}, {0.f, 4.f}, Colors.Purple, false, playerBodyDef, playerShapeDef
    );
    entities = {player};
    camera.entityToFollow = player;
    // Tempoaray test entities
    entities.push_back(new Entity{world, {50.f, 0.5f}, {0.f, 0.5f}, Colors.GrassGreen, true});
    entities.push_back(new Entity{world, {0.5f, 10.f}, {-18.f, 11.f}, Colors.Gray, true});
    entities.push_back(new Entity{world, {0.5f, 10.f}, {18.f, 11.f}, Colors.Gray, true});
    b2BodyDef dynamicBodyDef = b2DefaultBodyDef();
    dynamicBodyDef.type = b2_dynamicBody;
    dynamicBodyDef.linearDamping = 0.5f;
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    entities.push_back(
        new Entity{world, {0.5f, 2.f}, {10.f, 3.f}, Colors.Brown, false, dynamicBodyDef, shapeDef}
    );
    entities.push_back(
        new Entity{world, {0.5f, 1.f}, {-10.f, 2.f}, Colors.Brown, false, dynamicBodyDef, shapeDef}
    );
}

void Platformer::handleSdlEvent(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        switch (event.type) {
        case SDL_EVENT_QUIT: {
            running = false;
            break;
        }
        case SDL_EVENT_WINDOW_RESIZED: {
            window.handleResize(event.window.data1, event.window.data2);
            break;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            auto verbResult = getBindingFromScancode(event.key.scancode);
            if (verbResult.has_value()) {
                InputVerbInfo& binding = verbResult.value();
                if (binding.activateOnRepeat || !event.key.repeat) {
                    InputState state =
                        event.type == SDL_EVENT_KEY_DOWN ? InputState_Pressed : InputState_Released;
                    GameEvents::Input(binding.verb, state);
                }
            }
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
            if (event.wheel.integer_y > 0) {
                GameEvents::Input(InputVerb_ZoomIn, InputState_Pressed);
            } else if (event.wheel.integer_y < 0) {
                GameEvents::Input(InputVerb_ZoomOut, InputState_Pressed);
            }
            break;
        }
        }
    }
}

void Platformer::handleGameEvent(void) {
    GameEvent event;
    while (GameEvents::Poll(event)) {
        switch (event.type) {
        case GameEventTypes_CloseWindow: {
            running = false;
        }; break;
        case GameEventTypes_PlaySound: {

        }; break;
        case GameEventTypes_ToggleFullscreen: {
            window.toggleFullscreen();
        }; break;
        case GameEventTypes_Input: {
            if (event.input.state == InputState_Pressed) {
                switch (event.input.verb) {
                case InputVerb_ToggleFullscreen:
                    GameEvents::ToggleFullscreen();
                    break;
                case InputVerb_ZoomIn:
                    window.incrementScaleMultiplierBy(0.05f);
                    break;
                case InputVerb_ZoomOut:
                    window.incrementScaleMultiplierBy(-0.05f);
                    break;
                case InputVerb_ResetZoom:
                    window.resetScaleMultiplier();
                }
            }
            if (player) {
                controlEntity(event.input, *player);
            }
        }; break;
        }
    }
}

void Platformer::drawDebugUi(void) {
    ImGui::Begin("Debug Menu");
    ImGui::Text(
        "\nApplication average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
        ImGui::GetIO().Framerate
    );
    WindowDimensions offset = window.getOffsetPixels();
    WindowDimensions windowSizePixels = window.getSizePixels();
    b2Vec2 windowSizeWorld = window.getSizeWorld();
    float scaleFactor = window.getScaleFactor();
    ImGui::Text(
        "\nWindow:\nSize Pixels: %d, %d\nSize World: %.1f, %.1f\nRender Offset: %d, "
        "%d\nScale: %.2f (Factor %.2f)",
        windowSizePixels.x, windowSizePixels.y, windowSizeWorld.x, windowSizeWorld.y, offset.x,
        offset.y, window.scaleMultiplier, scaleFactor
    );
    if (player) {
        b2Vec2 position = b2Body_GetPosition(player->bodyId);
        b2Vec2 velocity = b2Body_GetLinearVelocity(player->bodyId);
        ImGui::Text(
            "\nPlayer:\nInput: %d %d %d %d\nPosition: %.2f, %.2f\nVelocity: %.2f, %.2f",
            player->movement[EntityMovement_Up], player->movement[EntityMovement_Down],
            player->movement[EntityMovement_Left], player->movement[EntityMovement_Right],
            position.x, position.y, velocity.x, velocity.y
        );
    }
    if (camera.entityToFollow) {
        b2Vec2 safeAreaSize = camera.getSafeAreaSize();
        b2Vec2 safeAreaValue = camera.getEntitySafeAreaValue();
        ImGui::Text(
            "\nSafe Area:\nSize: %.2f, %.2f\nRatio from Center: %.2f, %.2f", safeAreaSize.x,
            safeAreaSize.y, safeAreaValue.x, safeAreaValue.y
        );
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), window.sdlRenderer);
}

void Platformer::physicsStepHandler(void) {
    currentTime = SDL_GetTicks();
    float deltaTime = (float)(currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    }
    accumulator += deltaTime;
    while (accumulator >= physicsStep) {
        for (Entity* entity : entities) {
            if (entity) {
                entity->update();
            }
        }
        b2World_Step(world, physicsStep, 4);
        accumulator -= physicsStep;
    }
}

void Platformer::run(void) {
    lastTime = SDL_GetTicks();
    running = true;
    while (running) {
        handleSdlEvent();
        handleGameEvent();
        physicsStepHandler();
        window.clearFrame();
        drawDebugUi();
        camera.run();
        for (Entity* entity : entities) {
            entity->draw(&window);
        }
        SDL_RenderPresent(window.sdlRenderer);
    }
}

void Platformer::close(void) {
    b2DestroyWorld(world);
    for (Entity* entity : entities) {
        delete entity;
    }
    entities.clear();
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(window.sdlRenderer);
    SDL_DestroyWindow(window.sdlWindow);
    SDL_Quit();
}
