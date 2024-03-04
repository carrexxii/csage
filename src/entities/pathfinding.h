#ifndef ENTITIES_PATHFINDING_H
#define ENTITIES_PATHFINDING_H

#include "maths/types.h"
#include "map.h"

#define PATHFINDING_MAX_LOOKAHEAD 64

struct Path {
	Vec3i start;
	Vec3i end;
	int8 block_path_current;
	int8 block_path[PATHFINDING_MAX_LOOKAHEAD][3];
	int8 local_path_current;
	int8 local_path[PATHFINDING_MAX_LOOKAHEAD][3];
	bool complete;
};

void path_new(struct Path* path, struct Map* map);

#endif
