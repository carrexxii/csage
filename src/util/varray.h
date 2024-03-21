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

inline static struct VArray varray_new(isize elemc, isize elem_sz);
inline static void* varray_set(struct VArray* restrict arr, isize i, const void* restrict elem);
inline static void* varray_get(struct VArray* arr, isize i);
inline static isize varray_push(struct VArray* restrict arr, const void* restrict data);
inline static void  varray_resize(struct VArray** arr, isize new_sz);
inline static void  varray_free(struct VArray* arr);

// TODO: flags for slow grow/no grow/...
inline static struct VArray varray_new(isize elemc, isize elem_sz)
{
	assert(elemc > 0 && elem_sz > 0);
	struct VArray arr = {
		.data = smalloc(elemc*elem_sz),
		.len     = 0,
		.cap     = elemc,
		.elem_sz = elem_sz,
	};

	DEBUG(4, "[UTIL] Created new VArray with %d elements of size %d", arr.cap, arr.elem_sz);
	return arr;
}

inline static void* varray_set(struct VArray* arr, isize i, const void* elem)
{
	assert(i >= 0 && i < arr->len);

	memcpy(arr->data + i*arr->elem_sz, elem, arr->elem_sz);

	return arr->data + i*arr->elem_sz;
}

inline static void* varray_get(struct VArray* arr, isize i)
{
	assert(i >= 0 && i < arr->len);
	return arr->data + i*arr->elem_sz;
}

inline static isize varray_push(struct VArray* restrict arr, const void* restrict elem)
{
	if (arr->len >= arr->cap) {
		arr->cap *= VARRAY_SIZE_MULTIPLIER;
		arr->data = srealloc(arr->data, arr->cap*arr->elem_sz);
		DEBUG(4, "[UTIL] Resized VArray [len=%d; cap=%d; elem_sz=%dB]", arr->len, arr->cap, arr->elem_sz);
	}

	memcpy(arr->data + arr->len*arr->elem_sz, elem, arr->elem_sz);

	return arr->len++;
}

// TODO: improve
inline static isize varray_push_many(struct VArray* restrict arr, isize count, const void* restrict elem)
{
	assert(count > 0);
	isize fst = arr->len;
	for (int i = 0; i < count; i++)
		varray_push(arr, (byte*)elem + i*arr->elem_sz);

	return fst;
}

inline static void varray_reset(struct VArray* arr)
{
	arr->len = 0;
}

inline static void varray_free(struct VArray* arr)
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
