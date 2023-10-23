#include "util/minheap.h"
#include "map.h"
#include "pathfinding.h"

struct Path path_new(ivec3s start, ivec3s end)
{
	struct Path path = {
		.start = start,
		.end   = end,
	};

	/* Get the block path */
	struct MinHeap heap = minheap_new(3, sizeof(int));
	minheap_insert(&heap, (int[]){ 1 });
	minheap_insert(&heap, (int[]){ 3 });
	minheap_insert(&heap, (int[]){ 6 });
	minheap_insert(&heap, (int[]){ 5 });
	minheap_insert(&heap, (int[]){ 9 });
	minheap_insert(&heap, (int[]){ 8 });
	minheap_insert(&heap, (int[]){ 2 });
	minheap_insert(&heap, (int[]){ 11 });
	minheap_print(&heap);
	exit(0);

	/* Get the local path in the current block */

	return path;
}
