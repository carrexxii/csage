#ifndef ENTITIES_PLAYER_H
#define ENTITIES_PLAYER_H

#include "gfx/model.h"
#include "body.h"

struct Player {
	ID    model;
	mat4* matrix;
	struct Body    body;
	enum Direction move_dir;
};
extern struct Player player;

void player_init();
void player_update();
void player_free();

inline static void player_move_left_cb(bool kdown)  { player.move_dir = DIRECTION_LEFT;  }
inline static void player_move_right_cb(bool kdown) { player.move_dir = DIRECTION_RIGHT; }
inline static void player_move_up_cb(bool kdown)    { player.move_dir = DIRECTION_UP;    }
inline static void player_move_down_cb(bool kdown)  { player.move_dir = DIRECTION_DOWN;  }

#endif
