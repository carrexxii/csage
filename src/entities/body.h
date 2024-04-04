#ifndef ENTITIES_BODY_H
#define ENTITIES_BODY_H

#include "maths/types.h"
#include "gfx/sprite.h"
#include "components.h"

void bodies_update(isize count, struct Body* bodies);

void body_set_dir(struct Body* body, enum Direction d, bool set);

#endif

