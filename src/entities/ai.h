#ifndef ENTITIES_AI_H
#define ENTITIES_AI_H

enum AIState {
	STATE_NONE,
	STATE_IDLE,
	STATE_WANDER,
	STATE_PATHING,
};

struct AI {
	enum AIState state;
};

void ais_update();

#endif
