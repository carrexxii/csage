#include "SDL3/SDL.h"

#include "string.h"
#include "arena.h"
#include "varray.h"
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

/* -------------------------------------------------------------------- */

static struct Arena* enumerate_arena;
static bool enumerate_with_path;
static int cb_dir_enumerate(void* user_data, const char* restrict dir, const char* restrict file) {
	struct VArray* arr = user_data;
	String path;
	if (enumerate_with_path)
		path = string_new_join(2, (String[]){
			string_new(dir , -1, enumerate_arena),
			string_new(file, -1, enumerate_arena)
		}, STRING("/"), enumerate_arena);
	else
		path = string_new(file, -1, enumerate_arena);

	varray_push(arr, &path);
	return 1;
}

void dir_enumerate(char* path, bool with_path, struct VArray* arr, struct Arena* arena)
{
	assert(path && arr && arena);

	enumerate_with_path = with_path;
	enumerate_arena     = arena;
	SDL_EnumerateDirectory(path, cb_dir_enumerate, arr);
}
