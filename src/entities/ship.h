#ifndef SHIP_H
#define SHIP_H

#include "physics.h"

#define SHIP_TYPE_COUNT 8

typedef intptr ShipID;

enum ShipType {
	SHIPTYPE_NONE = 0,
	SHIPTYPE_1    = 1,
};

struct Ship {
	enum ShipType type;
	struct Body body;
	struct Thruster* thrusters;
	intptr mdli;
};

void ships_init();
ShipID ship_new(enum ShipType type);
void ship_add_body(ShipID ship, enum ShipType type);
// void ship_add_thruster();
void ships_update();
void ships_free();

#endif
