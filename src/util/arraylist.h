#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#define ARRAYLIST_ARRAY_SIZE 4*1024

struct ArrayListNode {
	struct ArrayListNode* next;
	uint16 emptyc;
	uint8 data[];
}; static_assert(sizeof(struct ArrayListNode) == 16, "struct ArrayListNode");

struct ArrayList {
	uint16 itemsz; /* # of bytes per item */
	uint16 itemc;  /* # of items per node */
	uint32 nodesz; /* # of bytes per node */
	struct ArrayListNode* head;
}; static_assert(sizeof(struct ArrayList) == 16, "struct ArrayList");

struct ArrayList arrlst_create(uint16 itemsz);
void* arrlst_add(struct ArrayList lst, uint32 id, void* newdata);
struct ArrayListNode* arrlst_add_node(struct ArrayList lst, struct ArrayListNode* node);
void* arrlst_find(struct ArrayList lst, uint32 id);
void  arrlst_remove(struct ArrayList lst, uint32 id);
void  arrlst_print(struct ArrayList lst);
void  arrlst_print_data(struct ArrayList lst, const char* fmt);
void  arrlst_free(struct ArrayList lst);

#endif
