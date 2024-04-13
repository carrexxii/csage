#include "maths/maths.h"
#include "components.h"
#include "body.h"

void bodies_update(isize count, Body* bodies)
{
	Body* body;
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

void body_set_dir(Body* body, DirectionMask dir, bool set)
{
	if (!body->moving)
		body->dir_mask = 0;

	DirectionMask old_mask = body->dir_mask;
	if (set)
		body->dir_mask |= dir;
	else
		body->dir_mask &= ~dir;

	body->moving        = !!body->dir_mask;
	body->changed_state = old_mask != body->dir_mask;
	if (!body->dir_mask)
		body->dir_mask = old_mask;
}

