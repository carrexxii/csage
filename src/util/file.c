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
	assert(file);

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
static bool          enumerate_with_path;
static bool          enumerate_with_ext;
static String        enumerate_filter;
static int cb_dir_enumerate(void* user_data, const char* restrict dir, const char* restrict file) {
	struct VArray* arr = user_data;
	String path;
	String dir_str  = string_new(dir , -1, enumerate_arena);
	String file_str = string_new(file, -1, enumerate_arena);
	if (enumerate_with_path)
		path = string_new_join(2, (String[]){ dir_str, file_str }, STRING("/"), enumerate_arena);
	else
		path = string_new(file, -1, enumerate_arena);

	if (enumerate_filter.data && string_endswith(path, enumerate_filter)) {
		if (!enumerate_with_ext)
			string_strip_ext(&path);
		varray_push(arr, &path);
	}
	return 1;
}

// TODO: maybe rewrite with two calls and no arena param
void dir_enumerate(const char* path, bool with_path, bool with_ext, String ext, struct VArray* arr, struct Arena* arena)
{
	assert(path && arr && arena);

	enumerate_with_path = with_path;
	enumerate_with_ext  = with_ext;
	enumerate_filter    = ext;
	enumerate_arena     = arena;
	SDL_EnumerateDirectory(path, cb_dir_enumerate, arr);
}
