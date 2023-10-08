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
	struct Thruster thruster;
	intptr mdli;
};

void ships_init();
ShipID ship_new(enum ShipType type);
struct Ship* ship_get(ShipID ship); // Temporary
void ships_update();
void ships_free();

#define STRING_OF_SHIPTYPE(_t)     \
	_t == SHIPTYPE_NONE? "block":   \
	_t == SHIPTYPE_1   ? "triangle": \
	"triangle"

#endif
