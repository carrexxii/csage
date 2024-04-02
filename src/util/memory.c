#include <stdlib.h>

/* Fix for asan: https://github.com/google/sanitizers/issues/89 */
int dlclose(void*);
int dlclose(void*) { return 0; }

/* TODO: Fix printout bug + add levels for bigger allocs */

static isize malloced = 0;

void* _smalloc(isize s, const char* file, int line, const char* fn)
{
	void* mem = malloc(s);
	if (!mem) {
		ERROR("[MEM] Malloc failed for %lu bytes", s);
		exit(1);
	} else {
		malloced += s;
		if (s >= DEBUG_MALLOC_MIN)
			DEBUG(2, "[MEM] Allocated %luB (%.2fkB) (%.2fMB total) in \"%s:%d:%s\"",
			      s, (double)s/1024.0, (double)malloced/1024.0/1024.0, file, line, fn);
		return mem;
	}
}

void* _scalloc(isize n, isize s, const char* file, int line, const char* fn)
{
	isize b = n*s;
	void* mem = calloc(n, s);
	if (!mem) {
		ERROR("[MEM] Calloc failed for %lu bytes", b);
		exit(1);
	} else {
		malloced += b;
		if (b >= DEBUG_MALLOC_MIN)
			DEBUG(2, "[MEM] Allocated %luB (%.2fkB) (%.2fkB total) in \"%s:%d:%s\"",
			      b, (double)b/1024.0, (double)malloced/1024.0, file, line, fn);
		return mem;
	}
}

void* _srealloc(void* mem, isize s, const char* file, int line, const char* fn)
{
	mem = realloc(mem, s);
	if (!mem) {
		ERROR("[MEM] Realloc failed for %lu bytes", s);
		exit(1);
	} else {
		if (s >= DEBUG_MALLOC_MIN)
			DEBUG(2, "[MEM] Reallocated %luB (%.2fkB/%.2fMB) (%.2fkB total) in \"%s:%d:%s\"",
			      s, (double)s/1024.0, (double)s/1024.0/1024.0, (double)malloced/1024.0, file, line, fn);
		return mem;
	}
}

void _sfree(void* mem, const char* file, int line, const char* fn)
{
	if (mem)
		free(mem);
	else
		ERROR("[MEM] Attempt to free NULL pointer in \"%s:%d:%s\"", file, line, fn);
}
