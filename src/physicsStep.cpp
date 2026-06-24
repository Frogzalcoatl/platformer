#include "physicsStep.hpp"

static uint64_t currentTime = 0;
static float accumulator = 0.f;
static const float physicsStep = 1.0f / 60.0f;
void handlePhysicsStep(b2WorldId world, Player* player) {
    currentTime = SDL_GetTicks();
    float deltaTime = (float)(currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    }
    accumulator += deltaTime;
    while (accumulator >= physicsStep) {
        player->update();
        b2World_Step(world, physicsStep, 4);
        accumulator -= physicsStep;
    }
}