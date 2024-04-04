#include "maths/maths.h"
#include "components.h"
#include "body.h"

void bodies_update(isize count, struct Body* bodies)
{
	struct Body*   body;
	struct Entity* e;
	Vec2 dir;
	for (int i = 0; i < count; i++) {
		body = &bodies[i];
		if (!body->moving)
			continue;

		dir = dir_of_mask(body->dir_mask);
		body->vel = multiply(dir, body->speed);
		if (!isnan(body->vel.x) && !isnan(body->vel.y))
			body->pos = add(body->pos, body->vel);
	}
}

/* -------------------------------------------------------------------- */

void body_set_dir(struct Body* body, enum Direction d, bool set)
{
	if (!body->moving)
		body->dir_mask = 0;

	enum Direction old_mask = body->dir_mask;
	if (set)
		body->dir_mask |= d;
	else
		body->dir_mask &= ~d;

	body->moving        = !!body->dir_mask;
	body->changed_state = old_mask != body->dir_mask;
	if (!body->dir_mask)
		body->dir_mask = old_mask;
}

