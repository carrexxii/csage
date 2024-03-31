#ifndef UTIL_VARRAY_H
#define UTIL_VARRAY_H

#define VARRAY_SIZE_MULTIPLIER 2

#define VARRAY_MAP(arr, fn) do {                                         \
		for (int i##__LINE__ = 0; i##__LINE__ < arr->len; i##__LINE__++) \
			fn(varray_get(arr, i##__LINE__));                            \
	} while (0)

// TODO: Add allocator parameters
struct VArray {
	byte* data;
	int len;
	int cap;
	int elem_sz;
};

static inline struct VArray varray_new(isize elemc, isize elem_sz);
static inline void* varray_set(struct VArray* restrict arr, isize i, void* restrict elem);
static inline void* varray_get(struct VArray* arr, isize i);
static inline isize varray_push(struct VArray* restrict arr, void* restrict data);
static inline void* varray_pop(struct VArray* arr);
static inline void  varray_resize(struct VArray** arr, isize new_sz);
static inline void  varray_free(struct VArray* arr);

// TODO: flags for slow grow/no grow/...
static inline struct VArray varray_new(isize elemc, isize elem_sz)
{
	assert(elemc > 0 && elem_sz > 0);
	struct VArray arr = {
		.data    = smalloc(elemc*elem_sz),
		.len     = 0,
		.cap     = elemc,
		.elem_sz = elem_sz,
	};

	DEBUG(4, "[UTIL] Created new VArray with %d elements of size %d", arr.cap, arr.elem_sz);
	return arr;
}

static inline void* varray_set(struct VArray* arr, isize i, void* elem)
{
	assert(i >= 0 && i < arr->len);

	memcpy(arr->data + i*arr->elem_sz, elem, arr->elem_sz);

	return arr->data + i*arr->elem_sz;
}

static inline void* varray_get(struct VArray* arr, isize i)
{
	assert(i >= 0 && i < arr->len);
	return arr->data + i*arr->elem_sz;
}

static inline void* varray_pop(struct VArray* arr)
{
	if (arr->len <= 0)
		return NULL;

	return arr->data + (--arr->len)*arr->elem_sz;
}

static inline isize varray_push(struct VArray* restrict arr, void* restrict elem)
{
	if (arr->len >= arr->cap) {
		arr->cap *= VARRAY_SIZE_MULTIPLIER;
		arr->data = srealloc(arr->data, arr->cap*arr->elem_sz);
		DEBUG(3, "[UTIL] Resized VArray [len=%d; cap=%d; elem_sz=%dB]", arr->len, arr->cap, arr->elem_sz);
	}

	memcpy(arr->data + arr->len*arr->elem_sz, elem, arr->elem_sz);

	return arr->len++;
}

// TODO: improve
static inline isize varray_push_many(struct VArray* restrict arr, isize count, void* restrict elem)
{
	assert(count > 0);
	isize fst = arr->len;
	for (int i = 0; i < count; i++)
		varray_push(arr, (byte*)elem + i*arr->elem_sz);

	return fst;
}

static inline bool varray_contains(struct VArray* arr, void* data)
{
	for (int i = 0; i < arr->len; i++)
		if (!memcmp(varray_get(arr, i), data, arr->elem_sz))
			return true;

	return false;
}

static inline void varray_reset(struct VArray* arr)
{
	arr->len = 0;
}

static inline void varray_free(struct VArray* arr)
{
	arr->cap = 0;
	arr->len = 0;
	sfree(arr->data);
}

static void varray_print(struct VArray* arr)
{
	printf("VArray: (%d elements of size %d; %d total capacity)", arr->len, arr->elem_sz, arr->cap);
	for (int i = 0; i < arr->cap; i++) {
		printf(i % 4 == 0? "\n": "");
		printf("\t[%d: %2ld] %d", i, (byte*)(arr->data + i*arr->elem_sz) - arr->data, *(int*)(arr->data + i*arr->elem_sz));
		if (i == arr->len)
			printf("*");
	}
	printf("\n");
}

#endif
