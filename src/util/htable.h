#ifndef UTIL_HASHTABLE_H
#define UTIL_HASHTABLE_H

#include "string.h"

/* TODO:
 *  - Add a resize function and/or copy function
 *  - Add a dynamic array for storing linked list elements and strings (arena allocator?)
 */

struct HPair {
	String key;
	int64  val;
	struct HPair* next;
};

struct HTable {
	isize cap;
	struct HPair pairs[];
};

static inline struct HTable* htable_new(isize count);
static inline uint htable_hash(struct HTable* htable, String str);
static inline struct HPair* htable_insert(struct HTable* htable, String key, int64 val);
static inline struct HPair* htable_get_pair(struct HTable* htable, String key);
static inline int64 htable_get(struct HTable* htable, String key);
static inline int64 htable_get_or_insert(struct HTable* htable, String key, int64 val);
static inline void  htable_print(struct HTable* htable);
static inline void  htable_free(struct HTable* htable);

/* -------------------------------------------------------------------- */

static inline struct HTable* htable_new(isize count)
{
	struct HTable* htable = scalloc(sizeof(struct HTable) + count*sizeof(struct HPair), 1);
	htable->cap = count;

	return htable;
}

/* PJW Hash Function: https://www.partow.net/programming/hashfunctions/ */
static inline uint htable_hash(struct HTable* htable, String str)
{
	uint hash = 1315423911;
	char* c = str.data;
	while (*c)
		hash ^= ((hash << 5) + (*c++) + (hash >> 2));

	return hash % htable->cap;
}

static inline struct HPair* htable_insert(struct HTable* htable, String key, int64 val)
{
	struct HPair* pair = &htable->pairs[htable_hash(htable, key)];
	/* Only search through the list if first slot is not available */
	if (pair->key.data) {
		do {
			/* Check for duplicate keys */
			if (!strncmp(pair->key.data, key.data, key.len)) // TODO: string_compare()
				break;
			if (!pair->next) {
				pair->next = smalloc(sizeof(struct HPair)); // TODO: Have extra array in hashtable for this
				pair = pair->next;
				pair->next = NULL;
				break;
			}
			pair = pair->next;
		} while (pair);
	}
	if (!pair->key.data)
		pair->key = string_copy(key, NULL); // TODO: arena
	pair->val = val;

	return pair;
}

/* Returns NULL if the key was not found */
static inline struct HPair* htable_get_pair(struct HTable* htable, String key)
{
	struct HPair* pair = &htable->pairs[htable_hash(htable, key)];
	if (!pair->key.len)
		return NULL;

	while (strncmp(pair->key.data, key.data, key.len)) {
		pair = pair->next;
		if (!pair)
			return NULL;
	}

	return pair;
}

/* Returns -1 if the key was not found */
static inline int64 htable_get(struct HTable* htable, String key) {
	struct HPair* pair = htable_get_pair(htable, key);
	return pair? pair->val
	           : -1;
}

/* Same as htable_get, but will insert the value if the key is not present */
static inline int64 htable_get_or_insert(struct HTable* htable, String key, int64 val) {
	struct HPair* pair = htable_get_pair(htable, key);
	return pair? pair->val
	           : htable_insert(htable, key, val)->val;
}

/* Returns 0 if the value was set or -1 if the key was not found */
static inline int htable_set(struct HTable* htable, String key, int64 val)
{
	// TODO: This should insert if its not there (ie, combine with htable_insert())
	struct HPair* pair = htable_get_pair(htable, key);
	if (pair)
		pair->val = val;
	else
		return -1;

	return 0;
}

static inline void htable_print(struct HTable* htable)
{
	fprintf(stderr, "Hashtable (capacity: %ld):\n", htable->cap);
	struct HPair* pair;
	for (int i = 0; i < htable->cap; i++) {
		pair = &htable->pairs[i];
		fprintf(stderr, "\t[%s: %ld]", pair->key.data? pair->key.data: "", pair->val);
		while (pair->next) {
			pair = pair->next;
			fprintf(stderr, " -> [%s: %ld]", pair->key.data, pair->val);
		}
		fprintf(stderr, "\n");
	}
}

static inline void htable_free(struct HTable* htable)
{
	sfree(htable);
}

#endif
