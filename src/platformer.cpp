#include "platformer.hpp"
#include "colors.hpp"
#include "events.hpp"
#include "playerInput.hpp"

Platformer::Platformer(void) {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -22.0f};
    world = b2CreateWorld(&worldDef);
    player = new Entity(world, {0.5f, 0.5f}, {0.f, 4.f}, Colors.Purple, false);
    entities = {player};
    running = false;
    lastTime = SDL_GetTicks();
    connectDefaultVerbMappings();
    // Tempoaray test entities
    entities.push_back(new Entity{world, {50.f, 0.5f}, {0.f, 0.5f}, Colors.GrassGreen, true});
    entities.push_back(new Entity{world, {0.5f, 10.f}, {-18.f, 11.f}, Colors.Gray, true});
    entities.push_back(new Entity{world, {0.5f, 10.f}, {18.f, 11.f}, Colors.Gray, true});
    entities.push_back(new Entity{world, {0.5f, 2.f}, {10.f, 3.f}, Colors.Brown, false});
    entities.push_back(new Entity{world, {0.5f, 1.f}, {-10.f, 2.f}, Colors.Brown, false});
}

void Platformer::handleSdlEvent(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            window.handleResize(event.window.data1, event.window.data2);
            continue;
        } else if ((event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) && !event.key.repeat) {
            auto verbResult = getVerbFromScancode(event.key.scancode);
            if (verbResult.has_value()) {
                InputVerb verb = verbResult.value();
                InputState state = event.type == SDL_EVENT_KEY_DOWN ? InputState_Pressed : InputState_Released;
                GameEventInput(verb, state);
            }
        }
    }
}

void Platformer::handleGameEvent(void) {
    GameEvent event;
    while (GameEventPoll(event)) {
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
            if (event.input.verb == InputVerb_ToggleFullscreen && event.input.state == InputState_Pressed) {
                GameEventToggleFullscreen();
            } else {
                if (player) {
                    handleInputEvent(event.input, *player);
                }
            }
        }; break;
        }
    }
}

void Platformer::drawDebugUi(void) {
    ImGui::Begin("Debug Menu");
    ImGui::Text("\nApplication average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    WindowDimensions offset = window.getOffset();
    WindowDimensions windowSize = window.getSize();
    int scaleFactor = window.getScaleFactor();
    ImGui::Text("\nWindow:\nSize: %d, %d\nRender Offset: %d, %d\nScale Factor: %d", windowSize.x, windowSize.y,
                offset.x, offset.y, scaleFactor);
    if (player) {
        b2Vec2 position = b2Body_GetPosition(player->bodyId);
        b2Vec2 velocity = b2Body_GetLinearVelocity(player->bodyId);
        ImGui::Text("\nPlayer:\nInput: %d %d %d %d\nPosition: %.2f, %.2f\nVelocity: %.2f, %.2f",
                    player->movement[EntityMovement_Up], player->movement[EntityMovement_Down],
                    player->movement[EntityMovement_Left], player->movement[EntityMovement_Right], position.x,
                    position.y, velocity.x, velocity.y);
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
        if (player) {
            player->update();
        }
        b2World_Step(world, physicsStep, 4);
        accumulator -= physicsStep;
    }
}

void Platformer::run(void) {
    running = true;
    while (running) {
        handleSdlEvent();
        handleGameEvent();
        physicsStepHandler();
        window.clearFrame();
        drawDebugUi();
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