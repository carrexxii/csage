#ifndef UTIL_FILE_H
#define UTIL_FILE_H

FILE*  file_open(const char* restrict path, const char* restrict mode);
intptr file_size(FILE* file);
char* const file_loadf(FILE* file);

inline static char* const file_load(const char* path) {
	return file_loadf(file_open(path, "rb"));
}

#endif
