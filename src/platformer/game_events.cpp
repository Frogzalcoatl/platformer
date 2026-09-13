#include "platformer/game_events.h"
#include <SDL3/SDL.h>
#include <array>
#include <queue>

constexpr size_t event_queue_size = 256;
static std::array<GameEvent, event_queue_size> event_queue = {};
static size_t head = 0;
static size_t tail = 0;
// Learned about the C++ syntax for a priority queue from AI
// But decided to use it myself bc I learned about it in data structures >:)
static std::
    priority_queue<ScheduledEvent, std::vector<ScheduledEvent>, std::greater<ScheduledEvent>>
        delayed_events;

bool game_events::poll(GameEvent& event) {
    if (head == tail) {
        return false;
    }
    event = std::move(event_queue[tail]);
    tail = (tail + 1) % event_queue_size;
    return true;
}

void game_events::push(GameEvent event) {
    size_t next_head = (head + 1) % event_queue_size;
    if (next_head == tail) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Event Queue Overflow. Dropping event type %zu",
            event.index()
        );
        return;
    }
    event_queue[head] = std::move(event);
    head = next_head;
}

void game_events::schedule(GameEvent event, Uint64 delay_ms) {
    Uint64 target_time = SDL_GetTicks() + delay_ms;
    delayed_events.push({target_time, std::move(event)});
}

void game_events::update_scheduled_events() {
    const Uint64 now = SDL_GetTicks();
    while (!delayed_events.empty() && delayed_events.top().execute_time_ms <= now) {
        game_events::push(delayed_events.top().event);
        delayed_events.pop();
    }
}