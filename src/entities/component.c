#include "component.h"
#include "common.h"

#define KB_PER_BLOCK 16
#define BLOCK_SIZE   (KB_PER_BLOCK*1024)

inline static void* alloc_new_block(struct ComponentArray* arr);

struct ComponentArray component_new(uint item_size)
{
	struct ComponentArray arr = {
		.item_size       = item_size,
		.items_per_block = (BLOCK_SIZE - sizeof(struct ComponentBlock))/(item_size + sizeof(uint16)),
	};
	alloc_new_block(&arr);

	DEBUG(5, "[ENT] Created new component array (item size: %u; items per block: %u)", item_size, arr.items_per_block);
	return arr;
}

void* component_add(struct ComponentArray* arr, uint16 id, void* restrict data)
{
	struct ComponentBlock* block;
	int i = -1;
	void* dst = NULL;
	for (block = arr->blocks; block; block = block->next) {
		/* Check if the block is already full */
		if (block->indc == arr->items_per_block)
			continue;

		/* TODO: support removals/unordered ids and preventing duplicate ids */
		i   = block->indc++;
		dst = block->data + i*arr->item_size;

		break;
	}

	if (!dst) {
		block = alloc_new_block(arr);
		dst   = block->data;
		i     = block->indc++;
	}
	block->inds[i] = id;
	memcpy(dst, data, arr->item_size);

	arr->itemc++;
	return dst;
}

void component_copy(struct ComponentArray* arr, void* mem)
{
	struct ComponentBlock* block;
	FOREACH_BLOCK(arr, block) {
		memcpy(mem, block->data, arr->item_size*block->indc);
		mem = (byte*)mem + arr->item_size*block->indc;
	}
}

void component_print(struct ComponentArray* arr)
{
	fprintf(stderr, "ComponentArray with %u items (item size: %u; items per block: %u)\n",
	        arr->itemc, arr->item_size, arr->items_per_block);
	struct ComponentBlock* block;
	FOREACH_BLOCK(arr, block) {
		fprintf(stderr, "Block (%u items):\n", block->indc);
		for (int i = 0; i < (int)arr->items_per_block; i++) {
			if (block->inds[i])
				fprintf(stderr, "\t[%3d (%.3d)] -> %p\n", i, block->inds[i], (void*)(block->data + i*arr->item_size));
		}
	}
}

void component_free(struct ComponentArray* arr, void (*fn)(void*))
{

}

inline static void* alloc_new_block(struct ComponentArray* arr)
{
	byte* mem = scalloc(sizeof(struct ComponentBlock) +
	                    arr->items_per_block*sizeof(uint16) +
	                    arr->items_per_block*arr->item_size, 1);
	byte* inds = mem  + sizeof(struct ComponentBlock);
	byte* data = inds + sizeof(uint16)*arr->items_per_block;

	struct ComponentBlock* block = arr->blocks;
	if (block) {
		for (; block->next; block = block->next);
		block->next = (struct ComponentBlock*)mem;
		block->next->inds = (uint16*)inds;
		block->next->data = data;

		return block->next;
	} else { /* First block */
		arr->blocks = (struct ComponentBlock*)mem;
		arr->blocks->inds = (uint16*)inds;
		arr->blocks->data = data;

		return arr->blocks;
	}
}
