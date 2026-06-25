#pragma once
#include "input.hpp"

enum GameEventTypes {
    GameEventTypes_None,
    GameEventTypes_CloseWindow,
    GameEventTypes_PlaySound,
    GameEventTypes_ToggleFullscreen,
    GameEventTypes_Input,
    GameEventTypes_Count
};

struct GameEvent {
    GameEventTypes type;
    union {
        struct {
            int soundId;
        } playSound;
        InputEvent input;
    };
};

namespace GameEvents {
bool Poll(GameEvent& event);
void Push(GameEvent event);
void CloseWindow();
void PlaySound(int soundId);
void ToggleFullscreen(void);
void Input(InputVerb inputVerb, InputState state);
} // namespace GameEvents