#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#define ARRAYLIST_ARRAY_SIZE 1*1024

/* Nodes are laid out with data as: [indices][data] */
struct ArrayListNode {
	struct ArrayListNode* next;
	uint16 emptyc;
	byte  data[];
}; static_assert(sizeof(struct ArrayListNode) == 16, "struct ArrayListNode");

struct ArrayList {
	uint16 itemsz; /* # of bytes per item */
	uint16 itemc;  /* # of items per node */
	uint32 nodesz; /* # of bytes per node */
	struct ArrayListNode* head;
}; static_assert(sizeof(struct ArrayList) == 16, "struct ArrayList");

struct ArrayList create_arrlst(uint16 itemsz);
void* arrlst_add(struct ArrayList lst, uint16 id, void* newdata);
void* arrlst_delete(struct ArrayList lst, uint16 id);
void* arrlst_get(struct ArrayList lst, uint16 id);
struct ArrayListNode* add_arrlst_node(struct ArrayList lst, struct ArrayListNode* node);
void print_arrlst(struct ArrayList lst);
void free_arrlst(struct ArrayList lst);

#endif
