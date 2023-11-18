#include "file.h"

FILE* file_open(char* restrict path, char* restrict mode)
{
	FILE* file = fopen(path, mode);
	fflush(file);
	if (!file)
		ERROR("[RES] Failed to open file: \"%s\"", path);
	else
		DEBUG(3, "[RES] Opened file \"%s\"", path);

	return file;
}

intptr file_size(FILE* file)
{
	intptr sz;
#if defined _WIN32
	sz = _filelengthi64(fileno(file));
#elif defined _POSIX_VERSION
	fseek(file, 0, SEEK_END);
	sz = ftell(file)*sizeof(char);
	rewind(file);
#endif

	return sz;
}

char* file_loadf(FILE* file)
{
	intptr sz = file_size(file);
	char* const buf = smalloc(sz + 1);
	fread(buf, 1, sz, file);
	buf[sz] = '\0';

	fclose(file);
	return buf;
}
