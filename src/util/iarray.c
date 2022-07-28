#include "iarray.h"

inline static intptr linear_search(uint16 indc, uint16* inds, uint16 key);
inline static intptr bin_search(uint16 indc, uint16* inds, uint16 key);

struct IArray create_iarr(uint16 itemsz, uint16 sz)
{
	struct IArray arr = {
		.sz     = sz? sz: DEFAULT_IARRAY_SIZE,
		.itemsz = itemsz,
		.sorted = true,
	};
	// ERROR("%d: %u", (bool)sz, sz? sz: DEFAULT_IARRAY_SIZE);
	resize_iarr(&arr, arr.sz);
	DEBUG(3, "[UTIL] Created IArray with %hu items of %hu size", sz, itemsz);

	return arr;
}

inline void resize_iarr(struct IArray* arr, uint16 sz)
{
	arr->inds = srealloc(arr->inds, sz*(sizeof(uint16) + arr->itemsz));
	arr->sz   = sz;
	arr->data = arr->inds + sz;
}

void iarr_append(struct IArray* arr, uint16 i, void* data)
{
	if (arr->itemc >= arr->sz)
		resize_iarr(arr, arr->sz*IARRAY_SIZE_MULTIPLIER);

	arr->inds[arr->itemc] = i;
	memcpy((byte*)arr->data + arr->itemc, data, arr->itemsz);
	if (arr->itemc && i < arr->inds[arr->itemc - 1])
		arr->sorted = false;
	arr->itemc++;
}

void* iarr_get(struct IArray arr, uint16 i)
{
	intptr arri;
	if (arr.sorted)
		arri = bin_search(arr.itemc, arr.inds, i);
	else
		arri = linear_search(arr.itemc, arr.inds, i);

	if (arri >= 0)
		return (byte*)arr.data + arri*arr.itemsz;
	else
		return NULL;
}

inline static intptr linear_search(uint16 indc, uint16* inds, uint16 key)
{
	for (uint i = 0; i < indc; i++)
		if (inds[i] == key)
			return i;
	return -1;
}

inline static intptr bin_search(uint16 indc, uint16* inds, uint16 key)
{
	uint l, u, k, i; /* lower, upper, key, index */
	l = 0;
	u = indc - 1;
	while (l <= u) {
		i = (l + u) / 2;
		k = inds[i];
		if (k == key)
			return i;
		else if (k > key)
			u = i - 1;
		else
			l = i + 1;
	}

	return -1;
}

