#ifndef UTIL_MINHEAP_H
#define UTIL_MINHEAP_H

#define MINHEAP_BUFFER_SIZE   256
#define MINHEAP_NODE_COUNT(d) ((1 << (d)) - 1)

struct MinHeap {
	int*  nodes;
	byte* data;
	int   data_size;
	int   depth;
	int   tail;
};

inline static void minheap_resize(struct MinHeap* heap, int depth)
{
	int nodec = MINHEAP_NODE_COUNT(depth);
	if (depth > heap->depth) {
		heap->depth = depth;
		heap->nodes = srealloc(heap->nodes, nodec*sizeof(*heap->nodes));
		heap->data  = srealloc(heap->data, (nodec + 1)*heap->data_size); /* +1 For returning data */
	} else {
		ERROR("[UTIL] Invalid new heap depth: got %d when at %d", depth, heap->depth);
	}
}

inline static struct MinHeap minheap_new(int depth, int data_size)
{
	struct MinHeap heap = {
		.data_size = data_size,
		.tail      = -1,
		.nodes     = NULL,
		.data      = NULL,
	};
	minheap_resize(&heap, depth);

	if (data_size >= DEBUG_MALLOC_MIN)
		DEBUG(5, "[UTIL] Created new MinHeap with depth %d (%d nodes of size %dB)",
		      depth, MINHEAP_NODE_COUNT(depth), data_size);
	return heap;
}

inline static int minheap_parent(int i) { return (i - 1)/2; }
inline static int minheap_left(int i)   { return 2*i + 1;   }
inline static int minheap_right(int i)  { return 2*i + 2;   }
inline static void* minheap_get(struct MinHeap* heap, int i) {
	return heap->data + i*heap->data_size;
}
inline static void minheap_swap(struct MinHeap* heap, int i1, int i2)
{
	if (i1 == i2)
		return;
	int tmp = heap->nodes[i1];
	heap->nodes[i1] = heap->nodes[i2];
	heap->nodes[i2] = tmp;

	byte tmp_buf[MINHEAP_BUFFER_SIZE];
	memcpy(tmp_buf, minheap_get(heap, i1), heap->data_size);
	memcpy(minheap_get(heap, i1), minheap_get(heap, i2), heap->data_size);
	memcpy(minheap_get(heap, i2), tmp_buf, heap->data_size);
}

inline static int minheap_min(struct MinHeap* heap) { return heap->nodes[0]; }
inline static void* minheap_pop(struct MinHeap* heap)
{
	void* data = minheap_get(heap, MINHEAP_NODE_COUNT(heap->depth));
	memcpy(data, heap->data, heap->data_size);

	int l, r, i = 0;
	minheap_swap(heap, 0, heap->tail);
	while (i <= heap->tail) {
		l = heap->nodes[minheap_left(i)];
		r = heap->nodes[minheap_right(i)];
		if (l < r && heap->nodes[i] < l) {
			minheap_swap(heap, i, minheap_left(i));
			i = minheap_left(i);
		} else if (r <= l && heap->nodes[i] < r) {
			minheap_swap(heap, i, minheap_right(i));
			i = minheap_right(i);
		} else {
			break;
		}
	}
	heap->tail--;

	return data;
}


inline static int minheap_push(struct MinHeap* heap, int val, void* data)
{
	int i = ++heap->tail;
	if (i >= MINHEAP_NODE_COUNT(heap->depth))
		minheap_resize(heap, heap->depth + 1);

	heap->nodes[i] = val;
	memcpy(minheap_get(heap, i), data, heap->data_size);
	while (i && heap->nodes[i] < heap->nodes[minheap_parent(i)]) {
		minheap_swap(heap, i, minheap_parent(i));
		i = minheap_parent(i);
	}

	return i;
}

inline static void* minheap_contains_data(struct MinHeap* heap, void* data)
{
	for (int i = 0; i <= heap->tail; i++)
		if (!memcmp(minheap_get(heap, i), data, heap->data_size))
			return minheap_get(heap, i);
	return NULL;
}

inline static int  minheap_size(struct MinHeap* heap)     { return heap->tail + 1; }
inline static bool minheap_is_empty(struct MinHeap* heap) { return heap->tail < 0; }
inline static void minheap_reset(struct MinHeap* heap)    { heap->tail = 0;        }

inline static void minheap_free(struct MinHeap* heap, void (*fn)(void*)) {
	sfree(heap->nodes);
	if (fn)
		for (int i = 0; i < heap->tail; i++)
			fn(minheap_get(heap, i));
	sfree(heap->data);
}

static void minheap_print(struct MinHeap* heap)
{
	DEBUG(1, "MinHeap with %d nodes (tail: %d)", MINHEAP_NODE_COUNT(heap->depth), heap->tail);
	int li, ri, l, r;
	for (int i = 0; i < MINHEAP_NODE_COUNT(heap->depth); i++) {
		li = minheap_left(i);
		ri = minheap_right(i);
		l  = heap->nodes[li];
		r  = heap->nodes[ri];
		fprintf(stderr, "[%d -> %d|%d] \t %d -> %d|%d \n", i, li, ri, i <= heap->tail? heap->nodes[i]: -1,
		        l && li <= heap->tail? l: -1, r && ri <= heap->tail? r: -1);
	}
	fprintf(stderr, "\n");
}

#endif
