#include "platformer.hpp"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
        SDL_Log("SDL Initialization failed: %s", SDL_GetError());
        return false;
    }
    Platformer game;
    game.run();
    game.close();
    return 0;
}