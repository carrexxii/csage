#include "maths/maths.h"
#include "util/minheap.h"
#include "map.h"
#include "pathfinding.h"

#define HEAP_STARTING_DEPTH    10
#define MAX_SEARCHED_POSITIONS 1024

struct PathNode {
	int g, h;
	Vec3i pos;
	struct PathNode* parent;
};

inline static int dist(Vec3i p1, Vec3i p2);
inline static void build_path(struct Path* path, int nodec, struct PathNode* nodes, bool is_local);

static Vec3i directions[] = {
	{  1, 0, 0 }, { -1,  0, 0 },
	{  0, 1, 0 }, {  0, -1, 0 },
	{  1, 1, 0 }, { -1, -1, 0 },
	{ -1, 1, 0 }, {  1, -1, 0 },
};

void path_new(struct Path* path, struct Map* map)
{
	Vec3i start = path->start;
	Vec3i end   = path->end;
	if (equal(start, end)) {
		ERROR("[ENT] Should not be trying to path with same start and end");
		return;
	}
	// if (!BETWEEN(end.x, 0, MAP_BLOCK_WIDTH  - 1) ||
	// 	!BETWEEN(end.y, 0, MAP_BLOCK_HEIGHT - 1) ||
	// 	!BETWEEN(end.z, 0, MAP_BLOCK_DEPTH  - 1)) {
	// 	ERROR("[ENT] Cannot find path to (%d, %d, %d)", end.x, end.y, end.z);
	// 	return;
	// }

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

	MapTile tile;
	struct PathNode  new_node;
	struct PathNode* old_node;
	do {
		current_node = *(struct PathNode*)minheap_pop(&open_nodes);
		closed_nodes[closed_nodec++] = current_node;
		if (!equal(current_node.pos, end)) {
			for (int i = 0; i < (int)ARRAY_SIZE(directions); i++) {
				// new_node.pos = add(current_node.pos, directions[i]);
				new_node.pos = VEC3I(current_node.pos.x + directions[i].x,
				                     current_node.pos.y + directions[i].y,
									 current_node.pos.z + directions[i].z);

				/* Skip if the cell is either map-blocked or already in closed_nodes */
				// TODO: update this for tilemap
				tile = map_get_tile(map, new_node.pos);
				if (!tile)
					goto skip_node;
				for (int j = 0; j < closed_nodec; j++)
					if (equal(closed_nodes[j].pos, new_node.pos))
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
	} while (!minheap_is_empty(&open_nodes) && closed_nodec < MAX_SEARCHED_POSITIONS);

	path->complete = false;
	build_path(path, closed_nodec, closed_nodes, true);

	// map_clear_highlight();
	// for (int i = 0; i < closed_nodec; i++) {
	// 	// DEBUG(1, "[%d] (%d, %d) = %d", i, path->local_path[i][0], path->local_path[i][1], closed_nodes[i].g+closed_nodes[i].h);
	// 	map_highlight_area((Recti){ path->local_path[i][0], path->local_path[i][1], 1, 1 });
	// }

	sfree(closed_nodes);
	minheap_free(&open_nodes, NULL);
}

inline static int dist(Vec3i p1, Vec3i p2)
{
	int dx = abs(p1.x - p2.x);
	int dy = abs(p1.y - p2.y);
	return dx > dy? 14*dy + 10*(dx - dy):
	                14*dx + 10*(dy - dx);
}

inline static void build_path(struct Path* path, int nodec, struct PathNode* nodes, bool is_local)
{
	if (is_local) {
		path->local_path_current = 0;
		memset(path->local_path, INT8_MIN, PATHFINDING_MAX_LOOKAHEAD*sizeof(path->local_path[0]));
		for (int i = 0; i < nodec; i++) {
			path->local_path[i][0] = nodes[i].pos.x;
			path->local_path[i][1] = nodes[i].pos.y;
			path->local_path[i][2] = nodes[i].pos.z;
		}
	} else {
		D;
	}
}
