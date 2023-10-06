#include "util/varray.h"

struct VArray varray_new(intptr elemc, intptr elem_size)
{
	if (elemc < 1)
		ERROR("[UTIL] Should not create VArray with < 1 elements");

	struct VArray arr = {
		.len       = elemc,
		.elem_size = elem_size,
		.data      = scalloc(elem_size, elemc),
	};

	DEBUG(4, "[UTIL] Created new VArray with %ld elements of size %ld (%ld total)", elemc, elem_size, elemc*elem_size);
	return arr;
}

void* varray_set(struct VArray* arr, intptr i, void* elem)
{
	if (i < 0) {
		ERROR("[UTIL] Array index cannot be negative (%ld)", i);
		return NULL;
	}
	if (i > arr->len)
		arr->len = i;
	if (i > arr->capacity)
		varray_resize(arr, -1);

	memcpy((byte*)arr->data + i*arr->elem_size, elem, arr->elem_size);

	return (byte*)arr->data + i*arr->elem_size;
}

void* varray_push(struct VArray* arr, void* elem)
{
	return varray_set(arr, arr->len, elem);
}

void varray_resize(struct VArray* arr, intptr new_size)
{
	intptr old_size = arr->capacity;
	if (new_size < 0)
		arr->capacity *= 1.5;
	else
		arr->capacity = new_size;
	arr->data = srealloc(arr->data, arr->capacity);

	DEBUG(3, "[UTIL] Resized VArray (%p) from %ld to %ld", (void*)arr, old_size, arr->capacity);
}

void varray_free(struct VArray* arr)
{
	arr->capacity = 0;
	arr->len      = 0;
	free(arr->data);
}
