#ifndef UTIL_STRINGS_H
#define UTIL_STRINGS_H

bool starts_with(const char* restrict str, const char* restrict start);
void file_extension(const char* restrict file, char* restrict name,
    char* restrict ext);

#endif
