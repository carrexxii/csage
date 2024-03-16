#include "player.h"

#include "maths/maths.h"
#include "gfx/sprite.h"

static isize sprite_sheet;
static struct Sprite* sprite;

static Vec3 dir;
static float speed = 0.1f;

void player_init()
{
	sprite_sheet = sprite_sheet_new("player");
	sprite       = sprite_new("player", "player", VEC3(0.0f, 0.0f, 1000.0f));

	DEBUG(1, "[ENT] Initialized player");
}

void player_set_moving(enum Direction d, bool set)
{
	switch (d) {
	case DIR_RIGHT: dir.x =  set; break;
	case DIR_LEFT : dir.x = -set; break;
	case DIR_UP   : dir.y = -set; break;
	case DIR_DOWN : dir.y =  set; break;
	default:
		ERROR("[ENT] Invalid direction for player movement: %d", d); // TODO: STRING_OF_DIRECTION
	}
}

void player_update()
{
	sprite->pos = add(sprite->pos, multiply(dir, speed));
}
