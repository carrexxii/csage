#ifndef UTIL_VARRAY_H
#define UTIL_VARRAY_H

#define VARRAY_SIZE_MULTIPLIER 2

#define VARRAY_MAP(arr, fn) do {                                         \
		for (int i##__LINE__ = 0; i##__LINE__ < arr->len; i##__LINE__++) \
			fn(varray_get(arr, i##__LINE__));                            \
	} while (0)

// TODO: Add allocator parameters
// TODO: VLS?
struct VArray {
	isize len;
	isize cap;
	isize elem_sz;
	byte* data;
};

static inline struct VArray varray_new(isize elemc, isize elem_sz);
static inline void* varray_set(struct VArray* restrict arr, isize i, void* restrict elem);
static inline void* varray_get(struct VArray* arr, isize i);
static inline isize varray_push(struct VArray* restrict arr, void* restrict data);
static inline isize varray_push_many(struct VArray* restrict arr, isize count, void* restrict elems);
static inline isize varray_push_many_strided(struct VArray* restrict arr, isize elemc, void* restrict elems, isize offset, isize stride);
static inline void* varray_pop(struct VArray* arr);
static inline bool  varray_contains(struct VArray* arr, void* data);
static inline void  varray_resize(struct VArray* arr, isize new_len, bool shrink);
static inline void  varray_free(struct VArray* arr);

/* -------------------------------------------------------------------- */

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

	DEBUG(3, "[UTIL] Created new VArray with %ld elements of size %ld", arr.cap, arr.elem_sz);
	return arr;
}

static inline void* varray_set(struct VArray* arr, isize i, void* elem)
{
	assert(i >= 0 && i < arr->len);

	memcpy(arr->data + i*arr->elem_sz, elem, arr->elem_sz);

	return arr->data + i*arr->elem_sz;
}

[[gnu::always_inline]]
static inline void* varray_get(struct VArray* arr, isize i)
{
	assert(i >= 0 && i < arr->len);

	return arr->data + i*arr->elem_sz;
}

static inline void* varray_pop(struct VArray* arr)
{
	assert(arr->len > 0);

	return arr->data + (--arr->len)*arr->elem_sz;
}

static inline isize varray_push(struct VArray* restrict arr, void* restrict elem)
{
	varray_resize(arr, arr->len + 1, false);
	memcpy(arr->data + arr->len*arr->elem_sz, elem, arr->elem_sz);

	return arr->len++;
}

static inline isize varray_push_many(struct VArray* restrict arr, isize elemc, void* restrict elems)
{
	assert(elemc > 0);

	varray_resize(arr, arr->len + elemc, false);
	memcpy(arr->data + arr->len*arr->elem_sz, elems, elemc*arr->elem_sz);
	arr->len += elemc;

	return arr->len - elemc;
}

static inline isize varray_push_many_strided(struct VArray* restrict arr, isize elemc, void* restrict elems, isize offset, isize stride)
{
	assert(elemc > 0);

	varray_resize(arr, arr->len + elemc, false);
	byte* data = (byte*)elems + offset;
	for (int i = 0; i < elemc; i++) {
		memcpy(arr->data + arr->len*arr->elem_sz, data, arr->elem_sz);
		data += stride;
		arr->len++;
	}

	return arr->len - elemc;
}

static inline bool varray_contains(struct VArray* arr, void* data)
{
	for (int i = 0; i < arr->len; i++)
		if (!memcmp(varray_get(arr, i), data, arr->elem_sz))
			return true;

	return false;
}

static inline void varray_resize(struct VArray* arr, isize new_len, bool shrink)
{
	if (arr->cap < new_len || (arr->cap != new_len && shrink)) {
		arr->cap  = new_len;
		arr->data = srealloc(arr->data, arr->cap * arr->elem_sz);
		DEBUG(3, "[UTIL] Resized VArray [len=%ld; cap=%ld; elem_sz=%ldB] = %ldB/%.2fkB/%.2fMB", arr->len, arr->cap, arr->elem_sz,
		      arr->cap*arr->elem_sz, arr->cap*arr->elem_sz/1024.0, arr->cap*arr->elem_sz/1024.0/1024.0);
	}
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
	printf("VArray: (%ld elements of size %ld; %ld total capacity)", arr->len, arr->elem_sz, arr->cap);
	for (int i = 0; i < arr->cap; i++) {
		printf(i % 4 == 0? "\n": "");
		printf("\t[%d: %2ld] %d", i, (byte*)(arr->data + i*arr->elem_sz) - arr->data, *(int*)(arr->data + i*arr->elem_sz));
		if (i == arr->len)
			printf("*");
	}
	printf("\n");
}

#endif
