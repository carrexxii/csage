#include "player.h"

#include "maths/maths.h"
#include "gfx/sprite.h"

static isize sprite_sheet;
static struct Sprite* sprite;

static uint dir;
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
	enum Direction old_dir = dir;
	if (set)
		dir |= d;
	else
		dir &= ~d;

	if      (d & DIR_N && dir & DIR_S) dir &= ~DIR_S;
	else if (d & DIR_S && dir & DIR_N) dir &= ~DIR_N;
	else if (d & DIR_W && dir & DIR_E) dir &= ~DIR_E;
	else if (d & DIR_E && dir & DIR_W) dir &= ~DIR_W;

	if (!dir)
		sprite_set_state(sprite, SPRITE_IDLE, d);
	else if (dir != old_dir && dir)
		sprite_set_state(sprite, SPRITE_RUN, dir);
}

void player_update()
{
	Vec2 vel;
	if      (dir & DIR_S && dir & DIR_E) vel = VEC2( 0.894427f,  0.447214f);
	else if (dir & DIR_S && dir & DIR_W) vel = VEC2(-0.894427f,  0.447214f);
	else if (dir & DIR_N && dir & DIR_E) vel = VEC2( 0.894427f, -0.447214f);
	else if (dir & DIR_N && dir & DIR_W) vel = VEC2(-0.894427f, -0.447214f);
	else if (dir & DIR_N) vel = VEC2( 0.0f, -1.0f);
	else if (dir & DIR_S) vel = VEC2( 0.0f,  1.0f);
	else if (dir & DIR_E) vel = VEC2( 1.0f,  0.0f);
	else if (dir & DIR_W) vel = VEC2(-1.0f,  0.0f);
	else vel = VEC2_ZERO;

	sprite->pos = VEC3_V2(add(VEC2_V3(sprite->pos), multiply(vel, speed)));
}
