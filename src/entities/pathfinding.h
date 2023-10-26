#ifndef ENTITIES_PATHFINDING_H
#define ENTITIES_PATHFINDING_H

#define PATHFINDING_MAX_LOOKAHEAD 64

struct Path {
	ivec3s start;
	ivec3s end;
	int8 block_path[PATHFINDING_MAX_LOOKAHEAD][3];
	int8 local_path[PATHFINDING_MAX_LOOKAHEAD][3];
	bool complete;
};

void path_new(struct Path* path);

#endif
