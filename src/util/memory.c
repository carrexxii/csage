#include <stdlib.h>

/* TODO: Fix printout bug + add levels for bigger allocs */

static uintptr malloced = 0;

void* _smalloc(uintptr s, const char* restrict file, int line, const char* restrict fn)
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

void* _scalloc(uintptr n, uintptr s, const char* restrict file, int line, const char* restrict fn)
{
	uintptr b = n*s;
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

void* _srealloc(void* mem, uintptr n, const char* restrict file, int line, const char* restrict fn)
{
	mem = realloc(mem, n);
	if (!mem) {
		ERROR("[MEM] Realloc failed for %lu bytes", n);
		exit(1);
	} else {
		DEBUG(2, "[MEM] Reallocated %luB (%.2fkB) (%.2fkB total) in \"%s:%d:%s\"",
		      n, (double)n/1024.0, (double)malloced/1024.0, file, line, fn);
		return mem;
	}
}

void _sfree(void* restrict mem, const char* file, int line, const char* fn)
{
	if (!mem) {
		ERROR("[MEM] Attempt to free NULL pointer in \"%s:%d:%s\"", file, line, fn);
	} else {
		free(mem);
		mem = NULL;
	}
}
