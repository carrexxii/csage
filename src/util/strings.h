#ifndef UTIL_STRINGS_H
#define UTIL_STRINGS_H

char* string_new(char* src, intptr len);
bool  starts_with(char* restrict str, char* restrict start);
void  file_extension(char* restrict file, char* restrict name, char* restrict ext);

#define string_contains(a, b) _Generic(b, \
	char : string_contains_char,           \
	char*: string_contains_string)(a, b)
bool string_contains_char(char* restrict str, char c);
bool string_contains_string(char* restrict str, char* restrict sub);

#endif
