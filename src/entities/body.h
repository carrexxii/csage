#ifndef ENTITIES_BODY_H
#define ENTITIES_BODY_H

#include "common.h"
#include "maths/types.h"
#include "gfx/sprite.h"
#include "components.h"

void bodies_update(isize count, Body* bodies);

void body_set_dir(Body* body, DirectionMask dir, bool set);

#endif

