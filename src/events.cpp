#include "events.hpp"
#include <SDL3/SDL.h>
#include <array>

#define EventQueueSize 256

static std::array<GameEvent, EventQueueSize> eventQueue = {};
static int head = 0;
static int tail = 0;

void GameEventPush(GameEvent event) {
    int nextHead = (head + 1) % EventQueueSize;
    if (nextHead == tail) {
        SDL_Log("Event Queue Overflow. Dropping event type %d", event.type);
        return;
    }
    eventQueue[head] = event;
    head = nextHead;
}

bool GameEventPoll(GameEvent& event) {
    if (head == tail) {
        // Queue is empty
        return false;
    }
    event = eventQueue[tail];
    tail = (tail + 1) % EventQueueSize;
    return true;
}

void GameEventCloseWindow(bool androidRemoveTask) {
    GameEvent event;
    event.type = GameEventTypes_CloseWindow;
    event.closeWindow.androidRemoveTask = androidRemoveTask;
    GameEventPush(event);
}

void GameEventPlaySound(int soundId) {
    GameEvent event;
    event.type = GameEventTypes_PlaySound;
    event.playSound.soundId = soundId;
    GameEventPush(event);
}

void GameEventToggleFullscreen(void) {
    GameEvent event;
    event.type = GameEventTypes_ToggleFullscreen;
    GameEventPush(event);
}

void GameEventInput(InputVerb inputVerb, InputState state) {
    GameEvent event;
    event.type = GameEventTypes_Input;
    event.input = InputEvent{inputVerb, state};
    GameEventPush(event);
}
