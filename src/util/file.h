#ifndef UTIL_FILE_H
#define UTIL_FILE_H

#include "arena.h"
#include "varray.h"

FILE*  file_open(char* restrict path, char* restrict mode);
intptr file_size(FILE* file);
char*  file_loadf(FILE* file);

void dir_enumerate(char* path, struct VArray* arr, struct Arena* arena);

static inline char* file_load(char* path)
{
	return file_loadf(file_open(path, "rb"));
}

#endif
