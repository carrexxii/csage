#ifndef UTIL_VARRAY_H
#define UTIL_VARRAY_H

#define VARRAY_SIZE_MULTIPLIER 1.5

struct VArray {
	intptr len;
	intptr max_len;
	intptr data_size;
	byte*  data;
};

struct VArray varray_new(intptr starting_item_count, intptr data_size);
void* varray_set(struct VArray* arr, intptr i, void* data);
int   varray_push(struct VArray* arr, void* data);
void  varray_resize(struct VArray* arr, intptr new_size);
void  varray_free(struct VArray* arr);

#endif
