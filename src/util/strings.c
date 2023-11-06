#include "strings.h"

char* string_new(char* src, intptr len)
{
	char* str = smalloc((len + 1)*sizeof(char));
	strncpy(str, src, len + 1);

	return str;
}

bool starts_with(char* restrict str, char* restrict start)
{
	for (int i = 0; start[i] && str[i]; i++)
		if (str[i] != start[i])
			return false;
	return true;
}

void file_extension(char* restrict file, char* restrict name, char* restrict ext)
{
	while (*++file);
	while (*--file != '/');
	if (name) {
		while ((*name++ = *++file) != '.');
		*--name = '\0'; /* Remove the period */
	} else {
		while (*++file != '.');
	}
	while (*ext++ = *++file);
}
