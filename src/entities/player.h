#ifndef ENTITIES_PLAYER_H
#define ENTITIES_PLAYER_H

#include "maths/types.h"

void player_init(void);
void player_set_moving(enum Direction dir, bool set);
void player_update(void);

#endif
