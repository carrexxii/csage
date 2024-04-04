#ifndef UTIL_ARENA_H
#define UTIL_ARENA_H

#define ARENA_RESIZE_FACTOR 2
#define ARENA_ALIGN_TO      8
#define ARENA_ALIGN(x)      ((x) += (ARENA_ALIGN_TO - ((x) % ARENA_ALIGN_TO)) % ARENA_ALIGN_TO)

enum ArenaFlag {
	ARENA_RESIZEABLE = 0x01,
	ARENA_NO_ALIGN   = 0x02,
};

struct Arena {
	byte* top;
	isize cap;
	enum ArenaFlag flags;
	byte data[];
};

inline static struct Arena* arena_new(isize sz, enum ArenaFlag flags);
inline static void* arena_alloc(struct Arena* arena, isize sz);
inline static void  arena_reset(struct Arena* arena);
inline static void  arena_free(struct Arena* arena);

inline static struct Arena* arena_new(isize sz, enum ArenaFlag flags)
{
	if (sz <= 0)
		ERROR("[MEM] Should not create an arena with size <= 0 (%ld)", sz);

	if (!(flags & ARENA_NO_ALIGN))
		ARENA_ALIGN(sz);

	struct Arena* arena = smalloc(sizeof(struct Arena) + sz);
	arena->top   = arena->data;
	arena->cap   = sz;
	arena->flags = flags;

	DEBUG(3, "[MEM] Created new arena of size %ldB/%ldkB", sz, sz/1024);
	return arena;
}

inline static void* arena_alloc(struct Arena* arena, isize in_sz)
{
	assert(arena && in_sz > 0);

	isize sz = in_sz;
	if (!(arena->flags & ARENA_NO_ALIGN))
		ARENA_ALIGN(sz);

	isize avail = arena->cap - (arena->top - arena->data);
	if (sz > avail) {
		if (arena->flags & ARENA_RESIZEABLE) {
			arena->cap = MAX(arena->cap*ARENA_RESIZE_FACTOR, sz);
			arena = srealloc(arena, sizeof(struct Arena) + arena->cap);
		} else {
			ERROR("[MEM] Arena out of memory.\n\tAvailable: %ldB of %ldB\n\trequested: %ldB",
			      avail, arena->cap, in_sz);
			return NULL;
		}
	}

	arena->top += sz;
	return arena->top - sz;
}

inline static void arena_reset(struct Arena* arena) {
	arena->top = arena->data;
}

inline static void arena_free(struct Arena* arena) {
	sfree(arena);
}

#endif
