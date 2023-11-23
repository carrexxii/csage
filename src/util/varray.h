#ifndef UTIL_VARRAY_H
#define UTIL_VARRAY_H

#define VARRAY_SIZE_MULTIPLIER 2

#define VARRAY_MAP(arr, fn) do {                                         \
		for (int i##__LINE__ = 0; i##__LINE__ < arr->len; i##__LINE__++) \
			fn(varray_get(arr, i##__LINE__));                            \
	} while (0)

// TODO: Add allocator parameters
struct VArray {
	isize len;
	isize cap;
	isize elem_sz;
	byte data[];
};

inline static struct VArray* varray_new(isize itemc, isize elem_sz);
inline static void* varray_set(struct VArray* arr, isize i, void* elem);
inline static void* varray_get(struct VArray* arr, isize i);
inline static isize varray_push(struct VArray* arr, void* data);
inline static void  varray_resize(struct VArray* arr, isize new_sz);
inline static void  varray_free(struct VArray* arr);

inline static struct VArray* varray_new(isize itemc, isize elem_sz)
{
	struct VArray* arr = smalloc(sizeof(struct VArray) + itemc*elem_sz);
	arr->len     = 0;
	arr->cap     = itemc;
	arr->elem_sz = elem_sz;

	DEBUG(4, "[UTIL] Created new VArray with %ld elements of size %ld", arr->cap, arr->elem_sz);
	return arr;
}

inline static void* varray_set(struct VArray* arr, isize i, void* elem)
{
	if (i > arr->cap*arr->elem_sz || i > arr->cap)
		varray_resize(arr, -1);

	memcpy(arr->data + i*arr->elem_sz, elem, arr->elem_sz);

	return arr->data + i*arr->elem_sz;
}

inline static void* varray_get(struct VArray* arr, isize i)
{
	assert(i < arr->len);
	return arr->data + i*arr->elem_sz;
}

inline static isize varray_push(struct VArray* arr, void* elem)
{
	varray_set(arr, arr->len, elem);
	return arr->len++;
}

// TODO: improve
inline static isize varray_push_many(struct VArray* arr, isize count, void* elem)
{
	for (int i = 0; i < count; i++)
		varray_push(arr, (byte*)elem + i*arr->elem_sz);
	return arr->len;
}

inline static void varray_resize(struct VArray* arr, isize new_sz)
{
	if (new_sz < 0)
		arr->cap *= VARRAY_SIZE_MULTIPLIER;
	else
		arr->cap = new_sz;
	arr = srealloc(arr, sizeof(struct VArray) + arr->cap*arr->elem_sz);
}

inline static void varray_reset(struct VArray* arr)
{
	arr->len = 0;
}

inline static void varray_free(struct VArray* arr)
{
	arr->cap = 0;
	arr->len = 0;
	sfree(arr);
}

static void varray_print(struct VArray* arr)
{
	printf("VArray: (%lu elements of size %lu; %lu total capacity)", arr->len, arr->elem_sz, arr->cap);
	for (int i = 0; i < arr->cap; i++) {
		printf(i % 4 == 0? "\n": "");
		printf("\t[%d: %2ld] %d", i, (byte*)varray_get(arr, i) - arr->data, *(int*)varray_get(arr, i));
		if (i == arr->len)
			printf("*");
	}
	printf("\n");
}

#endif
