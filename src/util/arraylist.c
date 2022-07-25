#include "arraylist.h"

inline static intptr binary_search_ids(uint16 idc, uint16* ids, uint16 key);

/* TODO: remove unused allocated data
         add option to specify amount of data allocated
         remove itemc duplication
*/
struct ArrayList create_arrlst(uint16 itemsz)
{
	struct ArrayList lst = {
		.nodesz = sizeof(struct ArrayListNode) + ARRLST_ARRAY_SIZE,
		.itemsz = itemsz,
		.itemc  = (uint16)(ARRLST_ARRAY_SIZE/(sizeof(uint16) + itemsz)),
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

	byte* data = ARRLST_DATA_START(lst, node) + i*lst.itemsz;
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

	/* Skip full blocks that the id shouldn't be in */
	uint16 lastitem;
	while (lastitem = ((uint16*)node->data)[lst.itemc - 1]) {
		if (node->next && lastitem < id)
			node = node->next;
		else
			break;
	}

	intptr i = -1;
	while (1) {
		i = binary_search_ids(lst.itemc - node->emptyc, (uint16*)node->data, id);
		if (i == -1) {
			if (node->next)
				node = node->next;
			else
				return NULL;
		} else {
			return ARRLST_DATA_START(lst, node) + i*lst.itemsz;
		}
	}
}

/* Remove the item from the ArrayList and return a pointer to its data */
void* arrlst_delete(struct ArrayList lst, uint16 id)
{
	struct ArrayListNode* node = lst.head;

	/* Skip full blocks that the id shouldn't be in */
	uint16 lastitem;
	while (lastitem = ((uint16*)node->data)[lst.itemc - 1]) {
		if (node->next && lastitem < id)
			node = node->next;
		else
			break;
	}

	intptr i = -1;
	while (1) {
		i = binary_search_ids(lst.itemc - node->emptyc, (uint16*)node->data, id);
		if (i == -1) {
			if (node->next) {
				node = node->next;
			} else {
				ERROR("[UTIL] Could not find item %hu to delete", id);
				return NULL;
			}
		} else {
			*(((uint16*)node->data) + i) = 0;
			return ARRLST_DATA_START(lst, node) + i*lst.itemsz;
		}
	}
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
	struct ArrayListNode* node = lst.head;
	struct ArrayListNode* nextnode;
	while (node->next) {
		nextnode = node->next;
		free(node);
		node = nextnode;
	}
	free(node);

	DEBUG(3, "[UTIL] Freed ArrayList (item count: %hu; item size: %hu)", lst.itemc, lst.itemsz);
}

void test_arraylist()
{
	DEBUG(1, TERM_MAGENTA "---> Testing ArrayList");
	float array[16];
	int   number = 777;
	char  string[64] = "Hello, World!";

	struct ArrayList lst;
	lst = create_arrlst(sizeof(string)); 
	DEBUG(1, "%u empty items, adding string \"Hello, World!\"...", lst.head->emptyc);
	arrlst_add(lst, 1, string);
	DEBUG(1, "%u empty items after adding", lst.head->emptyc);
	print_arrlst(lst);
	DEBUG(1, "Item with id 1 is: %s", (char*)arrlst_get(lst, 1));
	free_arrlst(lst);

	lst = create_arrlst(sizeof(number));
	DEBUG(1, "%u empty items, adding 200 random numbers...", lst.head->emptyc);
	uint k[200];
	srand(number);
	for (uint i = 1; i < 201; i++) {
		k[i-1] = rand() % 100;
		arrlst_add(lst, i, k + i - 1);
	}
	DEBUG(1, "%u empty items after adding and %u in next block", lst.head->emptyc, lst.head->next->emptyc);
	print_arrlst(lst);
	DEBUG(1, "\tItem with id 1   is: %u (should be: %u)", *(uint*)arrlst_get(lst, 1), k[0]);
	DEBUG(1, "\tItem with id 65  is: %u (should be: %u)", *(uint*)arrlst_get(lst, 65), k[64]);
	DEBUG(1, "\tItem with id 47  is: %u (should be: %u)", *(uint*)arrlst_get(lst, 47), k[46]);
	DEBUG(1, "\tItem with id 132 is: %u (should be: %u)", *(uint*)arrlst_get(lst, 132), k[131]);
	DEBUG(1, "\tItem with id 250 is: %p (should be: NULL)", (void*)arrlst_get(lst, 250));

	DEBUG(1, "Removing items at: 4, 64, 17 and 121...");
	DEBUG(1, "\tRemoved %u (should be %u)", *(uint*)arrlst_delete(lst, 4), k[3]);
	DEBUG(1, "\tRemoved %u (should be %u)", *(uint*)arrlst_delete(lst, 64), k[63]);
	DEBUG(1, "\tRemoved %u (should be %u)", *(uint*)arrlst_delete(lst, 17), k[16]);
	DEBUG(1, "\tRemoved %u (should be %u)", *(uint*)arrlst_delete(lst, 121), k[120]);
	print_arrlst(lst);
	free_arrlst(lst);
}

inline static intptr binary_search_ids(uint16 idc, uint16* ids, uint16 key)
{
	uint l, u, k, i; /* lower, upper, key, index */
	l = 0;
	u = idc - 1;
	while (l <= u) {
		i = (l + u) / 2;
		k = ids[i];
		if (k == key)
			return i;
		else if (k > key)
			u = i - 1;
		else
			l = i + 1;
	}

	return -1;
}
