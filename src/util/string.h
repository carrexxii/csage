#ifndef UTIL_STRINGS_H
#define UTIL_STRINGS_H

#include "arena.h"

// TODO: Add an allocator parameter for creating strings
typedef struct {
	isize len;
	char* data;
} String;

#define STRING(x) (String){ .data = x, .len = sizeof(x) - 1 }

String string_new(char* src, isize len, struct Arena* arena);
String string_new_join(isize strc, String* strs, String sep, struct Arena* arena);
String string_new_split(char* src, char sep, int index, struct Arena* arena);
String string_copy(String src, struct Arena* arena);
int    string_blit_cstr(char* restrict dst, char* restrict src, isize max_len);
int    string_contains(String str, char c);
int    string_remove(String str, char c);
void   string_free(String* str);

char* string_file_name(String path);

bool starts_with(char* restrict str, char* restrict start);
void file_extension(char* restrict file, char* restrict name, char* restrict ext);

#endif
