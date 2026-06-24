#pragma once
#include "player.hpp"

void handlePhysicsStep(b2WorldId world, Player* player);
inline uint64_t lastTime = 0;