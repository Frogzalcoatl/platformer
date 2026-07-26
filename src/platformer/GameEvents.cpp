#include "platformer/GameEvents.hpp"
#include <SDL3/SDL.h>
#include <array>

constexpr size_t EventQueueSize = 256;
static std::array<GameEvent, EventQueueSize> eventQueue = {};
static size_t head = 0;
static size_t tail = 0;
// Learned about the C++ syntax for a priority queue from AI
// But decided to use it myself bc I learned about it in data structures >:)
static std::
    priority_queue<ScheduledEvent, std::vector<ScheduledEvent>, std::greater<ScheduledEvent>>
        delayedEvents;

bool GameEvents::Poll(GameEvent& event) {
    if (head == tail) {
        // Queue is empty
        return false;
    }
    event = std::move(eventQueue[tail]);
    tail = (tail + 1) % EventQueueSize;
    return true;
}

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

void GameEvents::Schedule(GameEvent event, Uint64 delayMS) {
    Uint64 targetTime = SDL_GetTicks() + delayMS;
    delayedEvents.push({targetTime, std::move(event)});
}

void GameEvents::UpdateScheduledEvents() {
    const Uint64 now = SDL_GetTicks();
    while (!delayedEvents.empty() && delayedEvents.top().executeTimeMS <= now) {
        GameEvents::Push(delayedEvents.top().event);
        delayedEvents.pop();
    }
}
