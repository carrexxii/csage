#ifndef UTIL_VARRAY_H
#define UTIL_VARRAY_H

struct VArray {
	intptr elem_size;
	intptr capacity;
	intptr len;
	void*  data;
};

struct VArray varray_new(intptr elemc, intptr elem_size);
void* varray_set(struct VArray* arr, intptr i, void* elem);
void* varray_push(struct VArray* arr, void* elem);
void  varray_resize(struct VArray* arr, intptr new_size);
void  varray_free(struct VArray* arr);

#endif
