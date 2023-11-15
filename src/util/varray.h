#ifndef UTIL_VARRAY_H
#define UTIL_VARRAY_H

#define VARRAY_SIZE_MULTIPLIER 2

// TODO: Add allocator parameters
struct VArray {
	isize len;
	isize cap;
	isize elem_sz;
	byte data[];
};

inline static struct VArray* varray_new(intptr itemc, intptr elem_sz);
inline static void* varray_set(struct VArray* arr, intptr i, void* elem);
inline static void* varray_get(struct VArray* arr, intptr i);
inline static int   varray_push(struct VArray* arr, void* data);
inline static void  varray_resize(struct VArray* arr, intptr new_sz);
inline static void  varray_free(struct VArray* arr);

inline static struct VArray* varray_new(intptr itemc, intptr elem_sz)
{
	if (itemc < 1) {
		ERROR("[UTIL] Should not create VArray with < 1 elements");
		return NULL;
	}

	struct VArray* arr = smalloc(sizeof(struct VArray) + itemc*elem_sz);
	arr->len     = 0;
	arr->cap     = itemc;
	arr->elem_sz = elem_sz;

	DEBUG(4, "[UTIL] Created new VArray with %ld elements of size %ld", arr->cap, arr->elem_sz);
	return arr;
}

inline static void* varray_set(struct VArray* arr, intptr i, void* elem)
{
	if (i < 0 || i > arr->cap*VARRAY_SIZE_MULTIPLIER || !elem) {
		ERROR("[UTIL] Error setting value (%p) for varray (len: %ld; cap: %ld, i: %ld)",
		      elem, arr->len, arr->cap, i);
		return NULL;
	}
	if (i > arr->cap*arr->elem_sz || i > arr->cap)
		varray_resize(arr, -1);

	memcpy(arr->data + i*arr->elem_sz, elem, arr->elem_sz);

	return arr->data + i*arr->elem_sz;
}

inline static void* varray_get(struct VArray* arr, intptr i)
{
	return arr->data + i*arr->elem_sz;
}

inline static int varray_push(struct VArray* arr, void* data)
{
	varray_set(arr, arr->len, data);

	return arr->len++;
}

inline static void varray_resize(struct VArray* arr, intptr new_sz)
{
	if (new_sz < 0)
		arr->cap *= VARRAY_SIZE_MULTIPLIER;
	else
		arr->cap = new_sz;
	arr = srealloc(arr, sizeof(struct VArray) + arr->cap*arr->elem_sz);
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
