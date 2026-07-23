#include "Events.hpp"
#include <SDL3/SDL.h>
#include <array>

constexpr size_t EventQueueSize = 256;
static std::array<GameEvent, EventQueueSize> eventQueue = {};
static size_t head = 0;
static size_t tail = 0;

void GameEvents::Push(GameEvent event) {
    size_t nextHead = (head + 1) % EventQueueSize;
    if (nextHead == tail) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Event Queue Overflow. Dropping event type %zu",
            event.index()
        );
        return;
    }
    eventQueue[head] = std::move(event);
    head = nextHead;
}

bool GameEvents::Poll(GameEvent& event) {
    if (head == tail) {
        // Queue is empty
        return false;
    }
    event = std::move(eventQueue[tail]);
    tail = (tail + 1) % EventQueueSize;
    return true;
}
