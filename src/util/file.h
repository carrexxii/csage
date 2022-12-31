#ifndef UTIL_FILE_H
#define UTIL_FILE_H

FILE*  file_open(char* restrict path, char* restrict mode);
intptr file_size(FILE* file);
char* const file_loadf(FILE* file);

inline static char* const file_load(char* path) {
	return file_loadf(file_open(path, "rb"));
}

#endif
