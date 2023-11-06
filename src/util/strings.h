#ifndef UTIL_STRINGS_H
#define UTIL_STRINGS_H

char* string_new(char* src, intptr len);
bool  starts_with(char* restrict str, char* restrict start);
void  file_extension(char* restrict file, char* restrict name, char* restrict ext);

#endif
