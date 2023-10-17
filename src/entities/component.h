#ifndef ENTITIES_COMPONENT_H
#define ENTITIES_COMPONENT_H

#define FOREACH_BLOCK(arr, block) for (block = arr->blocks; block; block = block->next)
#define FOREACH_COMPONENT(arr, index, item, code) do {                                       \
	struct ComponentArray* _a##__LINE__ = _Generic((arr), struct ComponentArray : &(arr),    \
	                                                      struct ComponentArray*:  (arr));   \
	struct ComponentBlock* _b##__LINE__;                                                     \
	FOREACH_BLOCK(_a##__LINE__, _b##__LINE__) {                                              \
		for (int _i##__LINE__ = 0; _i##__LINE__ < (int)_b##__LINE__->indc; _i##__LINE__++) { \
			index = _b##__LINE__->inds[_i##__LINE__];                                        \
			item  = (typeof(item))_b##__LINE__->data + _i##__LINE__;                         \
			do { code } while(0);                                                            \
		}                                                                                    \
	}                                                                                        \
} while (0);
#define FOREACH_COMPONENT2(arr1, arr2, index, pitem1, pitem2, code) do {                      \
	struct ComponentArray* _a1##__LINE__ = _Generic((arr1), struct ComponentArray : &(arr1),  \
	                                                        struct ComponentArray*:  (arr1)); \
	struct ComponentArray* _a2##__LINE__ = _Generic((arr2), struct ComponentArray : &(arr2),  \
	                                                        struct ComponentArray*:  (arr2)); \
	struct ComponentBlock* _b1##__LINE__;                                                     \
	struct ComponentBlock* _b2##__LINE__;                                                     \
	for (_b1##__LINE__ = _a1##__LINE__->blocks, _b2##__LINE__ = _a2##__LINE__->blocks;        \
	     _b1##__LINE__ && _b2##__LINE__;                                                      \
	     _b1##__LINE__ = _b1##__LINE__->next, _b2##__LINE__ = _b2##__LINE__->next) {          \
		for (int _i##__LINE__ = 0; _i##__LINE__ < (int)_b1##__LINE__->indc; _i##__LINE__++) { \
			if (_b1##__LINE__->inds[_i##__LINE__] != _b2##__LINE__->inds[_i##__LINE__]) {     \
				ERROR("[ENT] ComponentArrays are not consistent");                            \
				break;                                                                        \
			}                                                                                 \
			index = _b1##__LINE__->inds[_i##__LINE__];                                        \
			pitem1 = (typeof(pitem1))_b1##__LINE__->data + _i##__LINE__;                      \
			pitem2 = (typeof(pitem2))_b2##__LINE__->data + _i##__LINE__;                      \
			do { code } while(0);                                                             \
		}                                                                                     \
	}                                                                                         \
} while (0);

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
