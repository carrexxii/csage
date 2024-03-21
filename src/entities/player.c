#include "player.h"

#include "maths/maths.h"
#include "gfx/sprite.h"

static isize sprite_sheet;
static struct Sprite* sprite;

static uint dir_mask;
static Vec2 dir;
static float speed = 0.1f;

void player_init()
{
	sprite_sheet = sprite_sheet_new("player", -2);
	sprite       = sprite_new(sprite_sheet, sprites_get_group(sprite_sheet, "player"), VEC3(0.0f, 0.0f, 0.0f));
	sprite_set_state(sprite, SPRITE_IDLE, DIR_S);

	DEBUG(1, "[ENT] Initialized player");
}

void player_set_moving(enum Direction d, bool set)
{
	uint old_mask = dir_mask;
	if (set)
		dir_mask |= d;
	else
		dir_mask &= ~d;

	dir = VEC2_ZERO;
	if (dir_mask & DIR_N) { dir.x -= 0.5f; dir.y -= 0.5f; }
	if (dir_mask & DIR_S) { dir.x += 0.5f; dir.y += 0.5f; }
	if (dir_mask & DIR_E) { dir.y -= 0.5f; dir.x += 0.5f; }
	if (dir_mask & DIR_W) { dir.y += 0.5f; dir.x -= 0.5f; }

	if (!dir_mask || dir_mask == (DIR_N | DIR_S)
	              || dir_mask == (DIR_E | DIR_W))
		sprite_set_state(sprite, SPRITE_IDLE, old_mask);
	else if (old_mask != dir_mask)
		sprite_set_state(sprite, SPRITE_RUN, dir_mask);
}

void player_update()
{
	Vec2 vel = multiply(normalized(dir), speed);
	if (!isnan(vel.x) && !isnan(vel.y))
		sprite->pos = add(sprite->pos, VEC3_V2(vel));
}
