#ifndef ENTITIES_PLAYER_H
#define ENTITIES_PLAYER_H

#include "maths/types.h"

extern struct Sprite* player_sprite;
extern Vec2  player_dir;
extern float player_speed;

void player_init(void);
void player_set_moving(enum Direction dir, bool set);
void player_update(void);

#endif
