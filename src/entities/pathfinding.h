#ifndef ENTITIES_PATHFINDING_H
#define ENTITIES_PATHFINDING_H

#define PATHFINDING_MAX_LOOKAHEAD 64

struct Path {
	ivec3s start;
	ivec3s end;
	int8 block_path[PATHFINDING_MAX_LOOKAHEAD];
	int8 local_path[PATHFINDING_MAX_LOOKAHEAD];
	bool complete;
};

struct Path path_new(ivec3s start, ivec3s end);

#endif
