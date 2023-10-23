#ifndef UTIL_MINHEAP_H
#define UTIL_MINHEAP_H

#define MINHEAP_BUF_SIZE      128
#define MINHEAP_NODE_COUNT(d) ((1 << (d)) - 1)

struct MinHeap {
	byte* nodes;
	int node_size;
	int depth;
	int tail;
};

inline static void minheap_resize(struct MinHeap* heap, int depth)
{
	if (depth > heap->depth) {
		heap->depth = depth;
		heap->nodes = srealloc(heap->nodes, MINHEAP_NODE_COUNT(heap->depth)*heap->node_size);
	} else {
		ERROR("[UTIL] Invalid new heap depth: got %d when at %d", depth, heap->depth);
	}
}

inline static struct MinHeap minheap_new(int depth, int node_size)
{
	struct MinHeap heap = {
		.node_size = node_size,
		.tail      = -1,
	};
	minheap_resize(&heap, depth);

	DEBUG(5, "[UTIL] Created new MinHeap with depth %d (%d nodes)", depth, MINHEAP_NODE_COUNT(depth));
	return heap;
}

inline static int minheap_parent(int i) { return (i - 1)/2; }
inline static int minheap_left(int i)   { return 2*i + 1;   }
inline static int minheap_right(int i)  { return 2*i + 2;   }
inline static void* minheap_get(struct MinHeap* heap, int i)
{
	if (i < MINHEAP_NODE_COUNT(heap->depth))
		return heap->nodes + i*heap->node_size;
	else
		return NULL;
}
inline static void minheap_swap(struct MinHeap* heap, int i1, int i2)
{
	assert(i1 != i2);
	byte tmp[MINHEAP_BUF_SIZE];
	void* p1 = minheap_get(heap, i1);
	void* p2 = minheap_get(heap, i2);
	memcpy(tmp, p1, heap->node_size);
	memcpy(p1, p2, heap->node_size);
	memcpy(p2, tmp, heap->node_size);
}

inline static int minheap_insert(struct MinHeap* heap, void* data)
{
	int i = ++heap->tail;
	if (i >= MINHEAP_NODE_COUNT(heap->depth))
		minheap_resize(heap, heap->depth + 1);

	memcpy(minheap_get(heap, i), data, heap->node_size);
	while (i && memcmp(minheap_get(heap, minheap_parent(i)), minheap_get(heap, i), heap->node_size) > 0) {
		minheap_swap(heap, i, minheap_parent(i));
		i = minheap_parent(i);
	}

	return i;
}

inline static int minheap_delete(struct MinHeap* heap, int i)
{
	assert(false);
}

inline static void minheap_free(struct MinHeap* heap, void (*free_fn)(void*)) {
	if (free_fn)
		for (int i = 0; i <= heap->tail; i++)
			free_fn(&heap->nodes[i]);
	free(heap->nodes);
}

inline static void minheap_print(struct MinHeap* heap)
{
	DEBUG(1, "MinHeap with %d nodes of size %d (tail: %d)", MINHEAP_NODE_COUNT(heap->depth), heap->node_size, heap->tail);
	int li, ri;
	int* l;
	int* r;
	for (int i = 0; i < MINHEAP_NODE_COUNT(heap->depth); i++) {
		li = minheap_left(i);
		ri = minheap_right(i);
		l = (int*)minheap_get(heap, li);
		r = (int*)minheap_get(heap, ri);
		fprintf(stderr, "[%d -> %d|%d] \t %d -> %d|%d \n", i, li, ri, i <= heap->tail? *(int*)minheap_get(heap, i): -1,
		        l && li <= heap->tail? *l: -1, r && ri <= heap->tail? *r: -1);
	}
	fprintf(stderr, "\n");
}

#endif
