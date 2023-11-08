#ifndef UTIL_STRINGS_H
#define UTIL_STRINGS_H

typedef struct {
	char*  data;
	intptr len;
} String;

typedef struct {
	char data[7];
	int8 len;
} String8;
typedef struct {
	char data[15];
	int8 len;
} String16;
typedef struct {
	char data[31];
	int8 len;
} String32;
typedef struct {
	char data[63];
	int8 len;
} String64;

String string_new(char* src, intptr len);
String string_copy(String src);
int    string_contains(String str, char c);

bool starts_with(char* restrict str, char* restrict start);
void file_extension(char* restrict file, char* restrict name, char* restrict ext);

#endif
