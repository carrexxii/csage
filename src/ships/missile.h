#ifndef SHIP_MISSILE_H
#define SHIP_MISSILE_H

#include "physics.h"
#include "gfx/model.h"
#include "ship.h"

enum MissileType {
	MISSILE_TYPE_NONE,
	MISSILE_TYPE_ONE,
};

struct Missile {
	enum MissileType type;
	struct Body      body;
	struct Model     model;
	ID     particles;
	ShipID owner;
	vec2   target;
	intptr mdli;
};

ID missile_new(enum MissileType type, vec2 start_pos, vec2 start_vel, ShipID owner);
void missile_set_target(ID missile_id, vec2 target);
void missiles_update();
void missiles_free();

#endif
