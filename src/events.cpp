#include "events.hpp"
#include <SDL3/SDL.h>
#include <array>

#define EventQueueSize 256

static std::array<GameEvent, EventQueueSize> eventQueue = {};
static int head = 0;
static int tail = 0;

void GameEvents::Push(GameEvent event) {
    int nextHead = (head + 1) % EventQueueSize;
    if (nextHead == tail) {
        SDL_Log("Event Queue Overflow. Dropping event type %d", event.type);
        return;
    }
    eventQueue[head] = event;
    head = nextHead;
}

bool GameEvents::Poll(GameEvent& event) {
    if (head == tail) {
        // Queue is empty
        return false;
    }
    event = eventQueue[tail];
    tail = (tail + 1) % EventQueueSize;
    return true;
}

void GameEvents::CloseWindow() {
    GameEvent event;
    event.type = GameEventTypes_CloseWindow;
    GameEvents::Push(event);
}

void GameEvents::PlaySound(int soundId) {
    GameEvent event;
    event.type = GameEventTypes_PlaySound;
    event.playSound.soundId = soundId;
    GameEvents::Push(event);
}

void GameEvents::ToggleFullscreen(void) {
    GameEvent event;
    event.type = GameEventTypes_ToggleFullscreen;
    GameEvents::Push(event);
}

void GameEvents::Input(InputVerb inputVerb, InputState state) {
    GameEvent event;
    event.type = GameEventTypes_Input;
    event.input = InputEvent{inputVerb, state};
    GameEvents::Push(event);
}
