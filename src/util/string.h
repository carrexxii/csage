#ifndef UTIL_STRINGS_H
#define UTIL_STRINGS_H

#include "arena.h"

typedef struct {
	isize len, cap;
	char* data;
} String;

#define STRING(x) (String){ .data = (x), .cap = sizeof(x), .len = sizeof(x) - 1 }

String string_new(char* src, isize cap, struct Arena* arena);
String string_new_join(isize strc, String* strs, String sep, struct Arena* arena);
String string_new_split(char* src, char sep, isize index, struct Arena* arena);
String string_copy(String src, struct Arena* arena);
int    string_blit_cstr(char* restrict dst, char* restrict src, isize max_len);
int    string_contains(String str, char c);
int    string_remove(String str, char c);
void   string_free(String* str);

void string_clear(String* str);
void string_cat_cstr(String* str1, char* str2, isize len);
bool string_endswith(String str, String ext);
bool string_strip_ext(String* str);

char* string_file_name(String path);

bool starts_with(char* restrict str, char* restrict start);
void file_extension(char* restrict file, char* restrict name, char* restrict ext);

#endif
