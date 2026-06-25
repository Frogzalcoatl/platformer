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
            bool androidRemoveTask;
        } closeWindow;
        struct {
            int soundId;
        } playSound;
        InputEvent input;
    };
};

bool GameEventPoll(GameEvent& event);
void GameEventPush(GameEvent event);
void GameEventCloseWindow(bool androidRemoveTask);
void GameEventPlaySound(int soundId);
void GameEventToggleFullscreen(void);
void GameEventInput(InputVerb inputVerb, InputState state);