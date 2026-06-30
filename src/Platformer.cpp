#include "Platformer.hpp"
#include "Colors.hpp"
#include "Drawing.hpp"
#include "Events.hpp"
#include "KeybindUi.hpp"
#include "PlayerInput.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

Platformer::Platformer()
    : window{"C++ Platformer", Colors.BackGround}, camera{Camera{nullptr, window}} {
    textEngine = TTF_CreateRendererTextEngine(window.sdlRenderer);
    if (!textEngine) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Unable to create SDL text engine: %s", SDL_GetError()
        );
    }
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -60.f};
    world = b2CreateWorld(&worldDef);
    camera.minViewableY = 0.f;
    updateKeybindsUi(input);
    // Test player
    b2BodyDef playerBodyDef = b2DefaultBodyDef();
    playerBodyDef.fixedRotation = false;
    b2ShapeDef playerShapeDef = b2DefaultShapeDef();
    playerShapeDef.density = 1.f;
    entities.push_back(
        std::make_unique<Entity>(
            world, b2MakeBox(0.5f, 0.5f), b2Vec2{0.f, 4.f}, hexToColor(0xBA988AFF), false,
            playerBodyDef, b2DefaultShapeDef()
        )
    );
    player = entities.front().get();
    camera.entityToFollow = player;
    // Tempoaray test entities
    entities.push_back(
        std::make_unique<Entity>(
            world, b2MakeBox(50.f, 0.5f), b2Vec2{0.f, 0.5f}, Colors.GrassGreen, true
        )
    );
    entities.push_back(
        std::make_unique<Entity>(
            world, b2MakeBox(0.5f, 10.f), b2Vec2{-18.f, 11.f}, Colors.Gray, true
        )
    );
    entities.push_back(
        std::make_unique<Entity>(
            world, b2MakeBox(0.5f, 10.f), b2Vec2{18.f, 11.f}, Colors.Gray, true
        )
    );
    b2BodyDef dynamicBodyDef = b2DefaultBodyDef();
    dynamicBodyDef.type = b2_dynamicBody;
    entities.push_back(
        std::make_unique<Entity>(
            world, b2MakeBox(0.5f, 2.f), b2Vec2{10.f, 3.f}, Colors.Brown, false, dynamicBodyDef,
            b2DefaultShapeDef()
        )
    );
    entities.push_back(
        std::make_unique<Entity>(
            world, b2MakeBox(0.5f, 1.f), b2Vec2{-10.f, 2.f}, Colors.Brown, false, dynamicBodyDef,
            b2DefaultShapeDef()
        )
    );
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
        case SDL_EVENT_KEY_UP: {
            std::vector<InputVerbInfo> verbs = input.getVerbsFromScancode(event.key.scancode);
            if (verbs.size() == 0) {
                break;
            }
            InputState state =
                event.type == SDL_EVENT_KEY_DOWN ? InputState::Pressed : InputState::Released;
            for (int i = 0; i < verbs.size(); i++) {
                if (verbs[i].activateOnRepeat || !event.key.repeat) {
                    GameEvents::Push(GameEventTypes::Input{verbs[i].verb, state});
                }
            }
        }; break;
        case SDL_EVENT_MOUSE_WHEEL: {
            if (event.wheel.integer_y > 0) {
                GameEvents::Push(GameEventTypes::Input{InputVerb::ZoomIn, InputState::Pressed});
            } else if (event.wheel.integer_y < 0) {
                GameEvents::Push(GameEventTypes::Input{InputVerb::ZoomOut, InputState::Pressed});
            }
        }; break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            std::vector<InputVerb> verbs = input.getVerbsFromGamepadButton(
                static_cast<SDL_GamepadButton>(event.gbutton.button)
            );
            if (verbs.size() == 0) {
                break;
            }
            const InputState state = event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN
                                         ? InputState::Pressed
                                         : InputState::Released;
            for (int i = 0; i < verbs.size(); i++) {
                GameEvents::Push(GameEventTypes::Input{verbs[i], state});
            }
        }; break;
        }
    }
}

static std::array<bool, static_cast<size_t>(InputVerb::Count)> inputThisFrame = {false};

void Platformer::handleGameEvent() {
    GameEvent event;
    while (GameEvents::Poll(event)) {
        if (std::holds_alternative<GameEventTypes::CloseWindow>(event)) {
            running = false;
        } else if (const auto* playSoundEvent = std::get_if<GameEventTypes::PlaySound>(&event)) {
            // Not finished
        } else if (const auto* inputEvent = std::get_if<GameEventTypes::Input>(&event)) {
            if (
                inputEvent->state == InputState::Pressed &&
                !inputThisFrame[static_cast<size_t>(inputEvent->verb)] // Prevent duplicate input
            ) {
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
                }
                inputThisFrame[static_cast<size_t>(inputEvent->verb)] = true;
            }
            if (player) {
                controlEntity(*inputEvent, *player, camera);
            }
        }
    }
    inputThisFrame.fill(false);
}

void Platformer::showDebugUi() const {
    ImGui::Begin("Debug Menu");
    ImGui::Text(
        "Renderer:\n%.1f/%s FPS (%.3f ms/frame)\nVsync Enabled: %s", ImGui::GetIO().Framerate,
        window.targetFpsStr().c_str(), 1000.0f / ImGui::GetIO().Framerate,
        window.isVsyncEnabled() ? "true" : "false"
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
        b2Vec2 position = b2Body_GetPosition(player->getBodyId());
        b2Vec2 velocity = b2Body_GetLinearVelocity(player->getBodyId());
        ImGui::Text(
            "\nPlayer:\nPosition: %.2f, %.2f\nVelocity: %.2f, %.2f\nInput: Up %d, Down %d, Left %d, Right %d, Sprint %d",
            position.x, position.y, velocity.x, velocity.y,
            player->movement[static_cast<size_t>(EntityMovement::Up)],
            player->movement[static_cast<size_t>(EntityMovement::Down)],
            player->movement[static_cast<size_t>(EntityMovement::Left)],
            player->movement[static_cast<size_t>(EntityMovement::Right)], player->isSprinting
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
}

void Platformer::physicsStepHandler() {
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
                entity->update();
            }
        }
        b2World_Step(world, physicsStep, 4);
        accumulator -= physicsStep;
    }
}

void Platformer::run() {
    TTF_Text* text =
        assets.getText(textEngine, "Player", GameAssets::Font::Monocraft, 150.f); // Just to test
    running = true;
    while (running) {
        const Uint64 frameStartNs = SDL_GetTicksNS();
        handleSdlEvent();
        handleGameEvent();
        physicsStepHandler();
        window.clearFrame();
        camera.run();
        for (const auto& entity : entities) {
            if (entity) {
                entity->draw(window);
            }
        }
        b2Vec2 textPos = player->getPosition();
        textPos.y += 2.f;
        Drawing::text(text, textPos, window);
        showDebugUi();
        showKeybindUi();
        window.render(frameStartNs);
    }
}

void Platformer::close() {
    entities.clear();
    b2DestroyWorld(world);
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(window.sdlRenderer);
    SDL_DestroyWindow(window.sdlWindow);
    SDL_Quit();
}
