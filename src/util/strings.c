#include "strings.h"

char* string_new(char* src, intptr len)
{
	if (len <= 0)
		len = strlen(src);

	char* str = smalloc((len + 1)*sizeof(char));
	strncpy(str, src, len);
	str[len] = '\0';

	return str;
}

bool string_contains_char(char* str, char c)
{
	int str_len = strlen(str);
	for (int i = 0; i < str_len; i++)
		if (str[i] == c)
			return true;
	return false;
}

// bool string_contains_string(char* restrict str, char* restrict sub)
// {
// 	int str_len = strlen(str);
// 	int sub_len = strlen(sub);
// 	bool has;
// 	for (int i = 0; i < str_len; i++) {
// 		has = true;
// 		for (int j = 0; j < sub_len; j++) {
// 			if (str[i + j] != sub[j]) {
// 				has = false;
// 				break;
// 			}
// 		}

// 		if (has)
// 			return true;
// 	}

// 	return false;
// }

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
