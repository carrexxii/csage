#ifndef UTIL_INDEXED_ARRAY_H
#define UTIL_INDEXED_ARRAY_H

#define DEFAULT_IARRAY_SIZE    10
#define IARRAY_SIZE_MULTIPLIER 1.5

struct IArray {
	intptr  sz;
	intptr  itemc;
	intptr  itemsz;
	bool    sorted;
	uint16* inds;
	void*   data;
};

struct IArray iarr_new(intptr itemsz, intptr sz);
void  iarr_resize(struct IArray* arr, intptr itemc);
void* iarr_append(struct IArray* arr, intptr i, void* data);
void* iarr_get(struct IArray arr, intptr i);
void  iarr_print(struct IArray arr);
void  iarr_free(struct IArray* arr, void (*cb)(void*));

#endif

