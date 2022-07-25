#include "arraylist.h"

#define DATA_START(_lst, _node) ((byte*)((_node)->data + (_lst).itemc*sizeof(uint16)))

/* TODO: remove unused allocated data
         add option to specify amount of data allocated
         remove itemc duplication
*/
struct ArrayList create_arrlst(uint16 itemsz)
{
	struct ArrayList lst = {
		.nodesz = sizeof(struct ArrayListNode) + ARRAYLIST_ARRAY_SIZE,
		.itemsz = itemsz,
		.itemc  = (uint16)(ARRAYLIST_ARRAY_SIZE/(sizeof(uint16) + itemsz)),
	};
	lst.head = scalloc(lst.nodesz, 1);
	lst.head->emptyc = lst.itemc;

	DEBUG(3, "[UTIL] Created ArrayList with item size %hu (%hu items per block)", lst.itemsz, lst.itemc);	
	return lst;
}

/* TODO:
 * - add error for adding with id 0
 */
void* arrlst_add(struct ArrayList lst, uint16 id, void* newdata)
{
	struct ArrayListNode* node = lst.head;
	while (node->emptyc == 0) {
		if (node->next)
			node = node->next;
		else
			node = add_arrlst_node(lst, node);
	}

	uint16* ids = (uint16*)node->data;
	uint i;
	for (i = 0; i < lst.itemc; i++) {
		if (!ids[i])
			break;
		else if (id == ids[i])
			ERROR("[UTIL] Replacing item with duplicate id (%u)", id);
	}

	byte* data = DATA_START(lst, node) + i*lst.itemsz;
	ids[i] = id;
	memcpy(data, newdata, lst.itemsz);
	node->emptyc--;
	
	return data;
}

/* Add a node to the ArrayList. If `node` is not NULL, then the new node will be added there. Otherwise, it will be at
 * the end of the list
 */
struct ArrayListNode* add_arrlst_node(struct ArrayList lst, struct ArrayListNode* node)
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

/* Returns a pointer to the data with index `id` */
void* arrlst_get(struct ArrayList const lst, uint16 id)
{
	struct ArrayListNode* node = lst.head;
	uint16* ids;
	byte*   data;

	/* Binary search for id in this block */
	uint l, u, k, i; /* lower, upper, key, index */
search_block:
	ids  = (uint16*)node->data;
	data = DATA_START(lst, node);
	l = 0;
	u = lst.itemc - node->emptyc - 1;
	while (l <= u) {
		i = (l + u) / 2;
		k = ids[i];
		DEBUG(1, "\t-> %u, %u, %u -> %u (%u)", l, u, i, k, id);
		if (k == id)
			return data + i*lst.itemsz;
		else if (k > id)
			l = i + 1;
		else
			u = i - 1;
	}

	if (node->next) {
		node = node->next;
		goto search_block;
	}

	return NULL;
}

void print_arrlst(struct ArrayList lst)
{
	uint16* ids;
	uint nodec = 0;
	struct ArrayListNode* node = lst.head;
print_node:
	ids = (uint16*)node->data;
	fprintf(stderr, "[%u] ", nodec++);
	for (uint i = 0; i < lst.itemc; i++)
		fprintf(stderr, "%hu ", ids[i]);
	if (node->next) {
		node = node->next;
		fprintf(stderr, "\n");
		goto print_node;
	}
	fprintf(stderr, "\n");
}

void free_arrlst(struct ArrayList lst)
{

}


void test_arraylist()
{
	DEBUG(1, TERM_MAGENTA "---> Testing ArrayList");
	float array[16];
	int   number = 777;
	char  string[64] = "Hello, World!";

	struct ArrayList lst;
	lst = create_arrlst(64); 
		DEBUG(1, "%u empty items, adding string \"Hello, World!\"...", lst.head->emptyc);
		arrlst_add(lst, 1, string);
		DEBUG(1, "%u empty items after adding", lst.head->emptyc);
		print_arrlst(lst);
		DEBUG(1, "Item with id 1 is: %s", (char*)arrlst_get(lst, 1));
	free_arrlst(lst);
}

