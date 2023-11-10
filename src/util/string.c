#include "string.h"

String* string_new(char* src, intptr len)
{
	if (len <= 0)
		len = strlen(src);
	String* str = smalloc(len + 1);
	str->len = len;

	memcpy(str->data, src, len);
	str->data[len] = '\0';

	return str;
}

String* string_copy(String* src)
{
	return string_new(src->data, src->len);
}

int string_contains(String* str, char c)
{
	char* s = str->data;
	for (int i = 0; i < str->len; i++)
		if (str->data[i] == c)
			return i;
	return -1;
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
