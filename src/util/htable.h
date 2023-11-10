#ifndef UTIL_HASHTABLE_H
#define UTIL_HASHTABLE_H

#include "string.h"

struct HPair {
	String32 key;
	union Data val;
	struct HPair* next;
};

struct HTable {
	intptr capacity;
	struct HPair pairs[];
};

inline static struct HTable* htable_new(intptr count)
{
	struct HTable* htable = scalloc(sizeof(struct HTable) + count*sizeof(struct HPair), 1);
	htable->capacity = count;

	return htable;
}

/* PJW Hash Function: https://www.partow.net/programming/hashfunctions/ */
inline static uint64 htable_hash(struct HTable* htable, String32 str)
{
	uint hash = 1315423911;
	char* c = str.data;
	while (*c)
		hash ^= ((hash << 5) + (*c++) + (hash >> 2));

	return hash % htable->capacity;
}

inline static struct HPair* htable_insert(struct HTable* htable, String32 key, union Data val)
{
	int i = htable_hash(htable, key);
	struct HPair* pair = &htable->pairs[i];
	if (pair->key.data[0]) {
		while (pair->next)
			pair = pair->next;
		pair->next = smalloc(sizeof(struct HPair)); // TODO: Have extra array in hashtable for this
		pair = pair->next;
	}
	pair->key  = key;
	pair->val  = val;
	pair->next = NULL;

	return pair;
}

/* Returns NULL if the key was not found */
inline static struct HPair* htable_get_pair(struct HTable* htable, String32 key)
{
	int i = htable_hash(htable, key);
	struct HPair* pair = &htable->pairs[i];
	while (strncmp(pair->key.data, key.data, sizeof(String32))) {
		pair = pair->next;
		if (!pair)
			return NULL;
	}

	return pair;
}

/* Returns DATA(0) if the key was not found */
inline static union Data htable_get(struct HTable* htable, String32 key)
{
	int i = htable_hash(htable, key);
	struct HPair* pair = htable_get_pair(htable, key);

	return pair? pair->val: DATA(0);
}

/* Returns 0 if the value was set or -1 if the key was not found */
inline static int htable_set(struct HTable* htable, String32 key, union Data val)
{
	int i = htable_hash(htable, key);
	struct HPair* pair = htable_get_pair(htable, key);
	if (pair)
		pair->val = val;
	else
		return -1;

	return 0;
}

inline static void htable_print(struct HTable* htable)
{
	fprintf(stderr, "Hashtable (capacity: %ld):\n", htable->capacity);
	struct HPair* pair;
	for (int i = 0; i < htable->capacity; i++) {
		pair = &htable->pairs[i];
		fprintf(stderr, "\t[%s: %ld]", pair->key.data, pair->val.s64);
		while (pair->next) {
			pair = pair->next;
			fprintf(stderr, " -> [%s: %ld]", pair->key.data, pair->val.s64);
		}
		fprintf(stderr, "\n");
	}
}

inline static void htable_free(struct HTable* htable)
{
	sfree(htable);
}

#endif
