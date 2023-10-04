#include "iarray.h"

inline static intptr linear_search(intptr indc, uint16* inds, intptr key);
inline static intptr bin_search(intptr indc, uint16* inds, intptr key);

struct IArray iarr_new(intptr itemsz, intptr sz)
{
	struct IArray arr = {
		.sz     = sz? sz: DEFAULT_IARRAY_SIZE,
		.itemsz = itemsz,
		.sorted = true,
	};
	iarr_resize(&arr, arr.sz);
	DEBUG(3, "[UTIL] Created IArray with %ld items of %ldB (%ldkB total)", sz, itemsz, sz*itemsz/1024);

	return arr;
}

void iarr_resize(struct IArray* arr, intptr sz)
{
	arr->inds = srealloc(arr->inds, sz*(sizeof(uint16) + arr->itemsz));
	arr->sz   = sz;
	arr->data = arr->inds + sz;
}

void* iarr_append(struct IArray* arr, intptr i, void* data)
{
	if (arr->itemc >= arr->sz)
		iarr_resize(arr, arr->sz*IARRAY_SIZE_MULTIPLIER);

	arr->inds[arr->itemc] = i;
	void* mem = (byte*)arr->data + arr->itemc*arr->itemsz;
	memcpy(mem, data, arr->itemsz);
	if (arr->itemc && i < arr->inds[arr->itemc - 1])
		arr->sorted = false;
	arr->itemc++;

	return mem;
}

/* Don't think this works */
void* iarr_get(struct IArray arr, intptr i)
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

inline static intptr linear_search(intptr indc, uint16* inds, intptr key)
{
	for (int i = 0; i < indc; i++)
		if (inds[i] == key)
			return i;

	return -1;
}

inline static intptr bin_search(intptr indc, uint16* inds, intptr key)
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

void iarr_print(struct IArray arr)
{
	fprintf(stderr, "IArray:\n\t");
	for (int i = 0; i < arr.itemc; i++) {
		fprintf(stderr, "%2hu ", arr.inds[i]);
		if (!((i + 1) % 20))
			fprintf(stderr, "\n\t");
	}
	fprintf(stderr, "\n");
}

void iarr_free(struct IArray* arr, void (*cb)(void*))
{
	if (cb)
		for (int i = 0; i < arr->itemc; i++)
			if (arr->inds[i])
				cb((byte*)arr->data + i*arr->itemsz);
	free(arr->inds); /* This also frees the data */
}
