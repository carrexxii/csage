#include "string.h"

String string_new(char* src, isize len)
{
	String str = {
		.data = smalloc(len + 1),
		.len  = len > 0? len: (isize)strlen(src),
	};
	memcpy(str.data, src, str.len);
	str.data[str.len] = '\0';

	return str;
}

String* string_new_ptr(char* src, isize len)
{
	// TODO:
	// String* str = smalloc(sizeof(String) + len + 1);
	// string_new(src, len, str->data);
	// return str;

	String* str = smalloc(sizeof(String) + len + 1);
	str->data = (void*)(str + 1);
	str->len = len > 0? len: (isize)strlen(src);
	memcpy(str->data, src, str->len);
	str->data[str->len] = '\0';

	return str;
}

/* Create a new string by splitting up `src` on `sep`s and using `index`.
 *   - If `index` is -1, the last part of the string is given.
 */
String string_new_split(char* src, char sep, int index)
{
	index = index == -1? INT_MAX: index;
	String str;

	char* start = src;
	int sepc = 0;
	int len  = 0;
	while (*src) {
		if (*src == sep) {
			sepc++;
			start = ++src;
			if (*start == sep)
				start++;
			if (sepc > index)
				return (String){ 0 };
		}
		if (sepc == index) {
			while (*src && *src++ != sep)
				len++;
			return len? string_new(start, len): (String){ 0 };
		}
		src++;
	}

	return string_new(start, 0);
}

String string_copy(String src)
{
	return string_new(src.data, src.len);
}

int string_contains(String str, char c)
{
	char* s = str.data;
	for (int i = 0; i < str.len; i++)
		if (str.data[i] == c)
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
