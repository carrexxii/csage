#include "player.h"

#include "maths/maths.h"
#include "gfx/sprite.h"

static isize sprite_sheet;
static struct Sprite* sprite;

static uint dir;
static float speed = 0.1f;

void player_init()
{
	sprite_sheet = sprite_sheet_new("player");
	sprite       = sprite_new("player", "player", VEC3(0.0f, 0.0f, -100.0f));

	DEBUG(1, "[ENT] Initialized player");
}

void player_set_moving(enum Direction d, bool set)
{
	if (set)
		dir |= d;
	else
		dir &= ~d;
}

void player_update()
{
	Vec3 vel;
	if      (dir & DIR_DOWN && dir & DIR_RIGHT) vel = VEC3( 0.894427f,  0.447214f, 0.0f);
	else if (dir & DIR_DOWN && dir & DIR_LEFT)  vel = VEC3(-0.894427f,  0.447214f, 0.0f);
	else if (dir & DIR_UP   && dir & DIR_RIGHT) vel = VEC3( 0.894427f, -0.447214f, 0.0f);
	else if (dir & DIR_UP   && dir & DIR_LEFT)  vel = VEC3(-0.894427f, -0.447214f, 0.0f);
	else if (dir & DIR_UP)    vel = VEC3( 0.0f, -1.0f, 0.0f);
	else if (dir & DIR_DOWN)  vel = VEC3( 0.0f,  1.0f, 0.0f);
	else if (dir & DIR_RIGHT) vel = VEC3( 1.0f,  0.0f, 0.0f);
	else if (dir & DIR_LEFT)  vel = VEC3(-1.0f,  0.0f, 0.0f);
	else vel = VEC3_ZERO;

	sprite->pos = add(sprite->pos, multiply(vel, speed));
}
