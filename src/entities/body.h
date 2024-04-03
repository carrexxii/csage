#ifndef ENTITIES_BODY_H
#define ENTITIES_BODY_H

#include "maths/types.h"
#include "gfx/sprite.h"

#define G 0.5f

struct Body {
	Vec2  pos;
	float speed;
	Vec2  vel;
	uint  dir_mask;
	Vec2  dir;
};

void bodies_update(isize count, struct Body* bodies);

void body_set_dir(struct Body* body, struct Sprite* sprite, enum Direction d, bool set);

#endif
