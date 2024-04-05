#ifndef UTIL_QUEUE_H
#define UTIL_QUEUE_H

struct Queue {
	isize elem_sz;
	isize cap;
	isize rear;
	isize front;
	byte data[];
};

static inline struct Queue* queue_new(isize len, isize elem_sz);
static inline void*         enqueue(struct Queue* restrict q, void* restrict data);
static inline void*         dequeue(struct Queue* q);
static inline void*         queue_peek(struct Queue* q);
static inline bool          queue_is_empty(struct Queue* q);
static inline void          queue_free(struct Queue* q);
static        void          queue_print(struct Queue* q);

/* -------------------------------------------------------------------- */

static inline struct Queue* queue_new(isize len, isize elem_sz)
{
	assert(elem_sz > 0 && len > 1);

	struct Queue* q = smalloc(sizeof(struct Queue) + len*elem_sz);
	*q = (struct Queue){
		.elem_sz = elem_sz,
		.cap     = len,
		.rear    = -1,
		.front   = -1,
	};

	return q;
}

static inline void* enqueue(struct Queue* restrict q, void* restrict data)
{
	assert(q && data);

	q->rear = (q->rear + 1) % q->cap;
	if (q->rear == q->front) {
		ERROR("Need to resize queue"); // TODO
		return NULL;
	}

	if (q->front == -1)
		q->front = 0;

	void* dst = q->data + q->rear*q->elem_sz; // FIXME: %
	memcpy(dst, data, q->elem_sz);
	return dst;
}

static inline void* dequeue(struct Queue* q)
{
	if (queue_is_empty(q))
		return NULL;

	void* data = q->data + q->front*q->elem_sz;
	if (q->front == q->rear) {
		q->front = -1;
		q->rear  = -1;
	} else {
		q->front = (q->front + 1) % q->cap;
	}

	return data;
}

static inline void* queue_peek(struct Queue* q)
{
	if (queue_is_empty(q))
		return NULL;

	return q->data + q->front*q->elem_sz;
}

static inline bool queue_is_empty(struct Queue* q)
{
	return q->front == -1;
}

static inline void queue_free(struct Queue* q)
{
	q->cap = 0;
	sfree(q);
}

static void queue_print(struct Queue* q)
{
	fprintf(stderr, "[UTIL] Queue with %ld capacity and elements of size %ld:\n\t", q->cap, q->elem_sz);
	for (int i = 0; i < q->cap; i++) {
		fprintf(stderr, "%ld", *(int64*)(q->data + i*q->elem_sz));
		if (i == q->front) fprintf(stderr, "$");
		if (i == q->rear)  fprintf(stderr, "^");
		if (i != q->cap - 1)
			fprintf(stderr, " -> ");
	}
	fprintf(stderr, "\n");
}

#endif

