#include "util/varray.h"

struct VArray varray_new(intptr starting_item_count, intptr data_size)
{
	if (starting_item_count < 1)
		ERROR("[UTIL] Should not create VArray with < 1 elements");

	struct VArray arr = {
		.len       = 0,
		.max_len   = starting_item_count*data_size,
		.data_size = data_size,
		.data      = scalloc(starting_item_count, data_size),
	};

	DEBUG(4, "[UTIL] Created new VArray with %ld elements of size %ld", arr.max_len, arr.data_size);
	return arr;
}

void* varray_set(struct VArray* arr, intptr i, void* data)
{
	if (i < 0 || i > arr->max_len*VARRAY_SIZE_MULTIPLIER || !data)
		ERROR("[UTIL] Error setting value (%p) for varray (len: %ld; max_len: %ld, i: %ld)",
		      data, arr->len, arr->max_len, i);
	if (i > arr->max_len*arr->data_size || i > arr->max_len)
		varray_resize(arr, -1);

	memcpy(arr->data + i*arr->data_size, data, arr->data_size);

	return arr->data + i*arr->data_size;
}

int varray_push(struct VArray* arr, void* data)
{
	varray_set(arr, arr->len, data);

	return arr->len++;
}

void varray_resize(struct VArray* arr, intptr new_size)
{
	if (new_size < 0)
		arr->max_len *= VARRAY_SIZE_MULTIPLIER;
	else
		arr->max_len = new_size;
	arr->data = srealloc(arr->data, arr->max_len*arr->data_size);
}

void varray_free(struct VArray* arr)
{
	arr->max_len = 0;
	arr->len     = 0;

	sfree(arr->data);
	arr->data = NULL;
}
