#ifndef ENTITIES_COMPONENT_H
#define ENTITIES_COMPONENT_H

struct ComponentBlock {
	struct ComponentBlock* next;
	uint16* inds;
	byte*   data;
	uint16  indc;
};

struct ComponentArray {
	uint item_size;
	uint items_per_block;
	uint itemc;
	struct ComponentBlock* blocks;
};

struct ComponentArray component_new(uint item_size);
void* component_add(struct ComponentArray* arr, uint16 id, void* data);
void  component_copy(struct ComponentArray* arr, void* mem);
void  component_print(struct ComponentArray* arr);
void  component_free(struct ComponentArray* arr, void (*fn)(void*));

#endif
