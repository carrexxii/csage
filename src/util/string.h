#ifndef UTIL_STRINGS_H
#define UTIL_STRINGS_H

// TODO: Add an allocator parameter for creating strings
typedef struct {
	isize len;
	char* data;
} String;

#define STRING(x) (String){ .data = x, .len = sizeof(x) - 1 }
#define STRING8(x)  (String8){  .data = x }
#define STRING16(x) (String16){ .data = x }
#define STRING32(x) (String32){ .data = x }
#define STRING64(x) (String64){ .data = x }
typedef struct { char data[8];  } String8;
typedef struct { char data[16]; } String16;
typedef struct { char data[32]; } String32;
typedef struct { char data[64]; } String64;

// TODO: Add allocator parameters
String  string_new(char* src, isize len);
String* string_new_cstr(char* src, isize len);
String  string_new_split(char* src, char sep, int index);
String  string_copy(String src);
int     string_blit_cstr(char* restrict dst, char* restrict src, isize max_len);
int     string_contains(String str, char c);
int     string_remove(String str, char c);
void    string_free(String str);

bool starts_with(char* restrict str, char* restrict start);
void file_extension(char* restrict file, char* restrict name, char* restrict ext);

#endif
