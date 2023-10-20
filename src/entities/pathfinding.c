#include "map.h"
#include "pathfinding.h"

struct Path path_new(ivec3s start, ivec3s end)
{
	struct Path path = {
		.start = start,
		.end   = end,
	};

	/* Get the block path */
	int blocki = map_get_block_index(start);

	/* Get the local path in the current block */

	return path;
}
