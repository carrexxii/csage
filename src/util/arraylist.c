#include "arraylist.h"

#define ID(_d)       ((uint16*)(_d))
#define DATA(_d)     ((_d) + sizeof(uint16))
#define NEXT(_l, _d) do { (_d) += (_l).itemsz + sizeof(uint16); } while(0)

/* TODO: remove unused allocated data
         add option to specify amount of data allocated
         remove itemc duplication
*/
struct ArrayList arrlst_create(uint16 itemsz)
{
	struct ArrayList lst = {
		.nodesz = sizeof(struct ArrayListNode) + ARRAYLIST_ARRAY_SIZE,
		.itemsz = itemsz,
		.itemc  = (uint16)(ARRAYLIST_ARRAY_SIZE/(sizeof(uint16) + itemsz)),
	};
	lst.head = calloc(lst.nodesz, 1);
	lst.head->emptyc = lst.itemc;
    if (ARRAYLIST_ARRAY_SIZE/(sizeof(uint16) + itemsz >= UINT16_MAX))
		ERROR("[UTIL] Arraylist should not have more than 2^15 elements, has %lu",
		      ARRAYLIST_ARRAY_SIZE/(sizeof(uint16) + itemsz));
    
    return lst;
}

/* TODO: add error for adding with id 0 */
void* arrlst_add(struct ArrayList lst, uint32 id, void* newdata)
{
	struct ArrayListNode* node = lst.head;
	while (node->emptyc == 0) {
next_node: /* this is in case the node found can't be used while keeping items in order */
		if (node->next)
			node = node->next;
		else
			node = arrlst_add_node(lst, node);
	}
	
	/* check to see if we can insert in this node whilst keeping it ordered */
	uint32 itemid;
	uint8* data = node->data;
	bool foundspot = false;
	for (uint i = 0; i < lst.itemc; i++) {
		itemid = *ID(data);
		if (itemid == id) {
			ERROR("[UTIL] Attempt to add item with duplicate id (%u)", id);
		} else if (!itemid) {
			foundspot = true;
			break;
		}
		NEXT(lst, data);
	}
	/* Couldn't insert in order here */
	if (!foundspot)
		goto next_node;

	*(ID(data)) = id;
	memcpy(DATA(data), newdata, lst.itemsz);
	node->emptyc--;
	
	return DATA(data);
}

struct ArrayListNode* arrlst_add_node(struct ArrayList lst, struct ArrayListNode* node)
{
	/* If no node was provided, find the last one */
	if (!node) {
		node = lst.head;
		while (node->next)
			node = node->next;
	}
		
	node->next = calloc(lst.nodesz, 1);
	node->next->emptyc = lst.itemc;
	
	return node->next;
}

void arrlst_print(struct ArrayList lst)
{
    uint nodec = 0;
	uint8* data;
	struct ArrayListNode* node = lst.head;
print_node:
	data = node->data;
	printf("[%u] ", nodec++);
	for (uint i = 0; i < lst.itemc; i++) {
		printf("%hu ", *ID(data));
		NEXT(lst, data);
	}
	if (node->next) {
		node = node->next;
		printf("\n");
		goto print_node;
	}
}

void arrlst_print_data(struct ArrayList lst, const char* fmt)
{
    uint nodec = 0;
	uint8* data;
	struct ArrayListNode* node = lst.head;
print_node:
	data = node->data;
	printf("Array %u\n", nodec++);
	for (uint i = 0; i < lst.itemc; i++) {
		printf("\t[%hu] ", *ID(data));
		printf(fmt, *DATA(data));
		printf("\n");
		NEXT(lst, data);
	}
	if (node->next) {
		node = node->next;
		goto print_node;
	}
}

