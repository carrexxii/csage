#include "maths/maths.h"
#include "gfx/sprite.h"
#include "entity.h"
#include "body.h"

void bodies_update(isize count, struct Body* bodies)
{
	struct Body*   body;
	struct Sprite* sprite;
	struct Entity* e;
	for (int i = 0; i < count; i++) {
		body = &bodies[i];
		body->vel = multiply(normalized(body->dir), body->speed);
		if (!isnan(body->vel.x) && !isnan(body->vel.y))
			body->pos = add(body->pos, body->vel);
	}
}

/* -------------------------------------------------------------------- */

void body_set_dir(struct Body* body, struct Sprite* sprite, enum Direction d, bool set)
{
	uint dir_mask = body->dir_mask;
	if (set)
		dir_mask |= d;
	else
		dir_mask &= ~d;

	Vec2 dir = VEC2_ZERO;
	if (dir_mask & DIR_N) { dir.x -= 0.5f; dir.y -= 0.5f; }
	if (dir_mask & DIR_S) { dir.x += 0.5f; dir.y += 0.5f; }
	if (dir_mask & DIR_E) { dir.y -= 0.5f; dir.x += 0.5f; }
	if (dir_mask & DIR_W) { dir.y += 0.5f; dir.x -= 0.5f; }

	if (!dir_mask || dir_mask == (DIR_N | DIR_S)
	              || dir_mask == (DIR_E | DIR_W))
		sprite_set_state(sprite, SPRITE_IDLE, body->dir_mask);
	else if (body->dir_mask != dir_mask)
		sprite_set_state(sprite, SPRITE_RUN, dir_mask);

	body->dir      = dir;
	body->dir_mask = dir_mask;
}
