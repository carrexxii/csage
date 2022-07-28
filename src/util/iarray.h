#ifndef UTIL_INDEXED_ARRAY_H
#define UTIL_INDEXED_ARRAY_H

#define DEFAULT_IARRAY_SIZE  10
#define IARRAY_SIZE_MULTIPLIER 1.5
#define IARRAY_MAP(arr, fn, ...) do {        \
	for (uint _i = 0; _i < (arr).itemc; _i++) \
		(fn)((arr).data[_i], __VA_ARGS__);     \
} while (0)

struct IArray {
	uint16  sz;
	uint16  itemc;
	uint16  itemsz;
	bool    sorted;
	uint16* inds;
	void*   data;
}; static_assert(sizeof(struct IArray) == 24, "struct IArray");

struct IArray create_iarr(uint16 itemsz, uint16 sz);
void  resize_iarr(struct IArray* arr, uint16 itemc);
void  iarr_append(struct IArray* arr, uint16 i, void* data);
void* iarr_get(struct IArray arr, uint16 i);

#endif
