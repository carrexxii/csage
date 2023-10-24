#include "util/minheap.h"
#include "map.h"
#include "pathfinding.h"

#define HEAP_STARTING_DEPTH    10
#define MAX_SEARCHED_POSITIONS 1024

struct PathNode {
	int g, h;
	ivec3s pos;
	struct PathNode* parent;
};

inline static int dist(ivec3s p1, ivec3s p2);

static ivec3s directions[] = {
	{  1, 0, 0 }, { -1,  0, 0 },
	{  0, 1, 0 }, {  0, -1, 0 },
	{  1, 1, 0 }, { -1, -1, 0 },
	{ -1, 1, 0 }, {  1, -1, 0 },
};

struct Path path_new(ivec3s start, ivec3s end)
{
	struct Path path = {
		.start = start,
		.end   = end,
	};

	if (start.z != end.z)
		ERROR("[ENT] Traversing z-levels is not completed");

	struct MinHeap   open_nodes   = minheap_new(HEAP_STARTING_DEPTH, sizeof(struct PathNode));
	struct PathNode* closed_nodes = smalloc(MAX_SEARCHED_POSITIONS*sizeof(struct PathNode));
	int closed_nodec = 0;

	struct PathNode current_node = {
		.pos    = start,
		.parent = NULL,
		.h      = dist(start, end),
	};
	minheap_push(&open_nodes, 0, &current_node);

	struct Voxel*    vxl;
	struct PathNode  new_node;
	struct PathNode* old_node;
	do {
		current_node = *(struct PathNode*)minheap_pop(&open_nodes);
		closed_nodes[closed_nodec++] = current_node;
		if (!ivec3s_eq(current_node.pos, end)) {
			for (int i = 0; i < (int)ARRAY_LEN(directions); i++) {
				new_node.pos = ivec3s_add(directions[i], current_node.pos);

				/* Skip if the cell is either map-blocked or already in closed_nodes */
				vxl = map_get_voxel(new_node.pos);
				if (!vxl || !vxl->data)
					goto skip_node;
				for (int j = 0; j < closed_nodec; j++)
					if (ivec3s_eq(closed_nodes[j].pos, new_node.pos))
						goto skip_node;

				new_node.g = current_node.h + (i <= 3? 10: 14); /* First 4 directions are orthogonal, last 4 are diagonal */
				new_node.h = dist(new_node.pos, end);

				old_node = minheap_contains_data(&open_nodes, &new_node);
				if (old_node && old_node->g < new_node.g) {
					old_node->g = new_node.g;
					old_node->parent = &current_node;
				} else {
					minheap_push(&open_nodes, new_node.g + new_node.h, &new_node);
				}

			skip_node:
			}
		} else {
			break;
		}
	} while (!minheap_is_empty(&open_nodes));

	for (int i = 0; i < closed_nodec; i++) {
		DEBUG(1, "[%d] (%d, %d) = %d", i, closed_nodes[i].pos.x, closed_nodes[i].pos.y, closed_nodes[i].g+closed_nodes[i].h);
		map_highlight_area((ivec4s){ closed_nodes[i].pos.x, closed_nodes[i].pos.y, 1, 1 });
	}

	free(closed_nodes);
	minheap_free(&open_nodes, NULL);

	return path;
}

inline static int dist(ivec3s p1, ivec3s p2)
{
	int dx = abs(p1.x - p2.x);
	int dy = abs(p1.y - p2.y);
	return dx > dy? 14*dy + 10*(dx - dy):
	                14*dx + 10*(dy - dx);
}
