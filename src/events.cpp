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
        SDL_Log("Event Queue Overflow. Dropping event type %zu", event.index());
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
