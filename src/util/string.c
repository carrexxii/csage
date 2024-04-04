#include "string.h"

/* Length is calculated if it is <= 0 */
String string_new(char* src, isize cap, struct Arena* arena)
{
	cap = cap > 0? cap: (isize)strlen(src);
	String str = {
		.data = arena? arena_alloc(arena, cap + 1): smalloc(cap + 1),
		.len  = src? cap: 0,
		.cap  = cap,
	};
	if (str.data) {
		if (src)
			memcpy(str.data, src, str.len);
		str.data[str.len] = '\0';
	}

	return str;
}

String string_new_join(isize strc, String* strs, String sep, struct Arena* arena)
{
	isize len = (strc - 1) * sep.len;
	for (int i = 0; i < strc; i++)
		len += strs[i].len;

	String str = {
		.data = arena? arena_alloc(arena, len + 1): smalloc(len + 1),
		.len  = len,
	};
	if (str.data) {
		char* head = str.data;
		for (int i = 0; i < strc; i++) {
			memcpy(head, strs[i].data, strs[i].len*sizeof(char));
			head += strs[i].len;
			if (i != strc - 1) {
				memcpy(head, sep.data, sep.len*sizeof(char));
				head += sep.len;
			}
		}
		*head = '\0';
	}

	return str;
}

/* Create a new string by splitting up `src` on `sep`s and using `index`.
 *   - If `index` is -1, the last part of the string is given.
 */
String string_new_split(char* src, char sep, isize index, struct Arena* arena)
{
	index = index == -1? INT64_MAX: index;

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
			return len? string_new(start, len, arena): (String){ 0 };
		}
		src++;
	}

	return string_new(start, 0, arena);
}

String string_copy(String src, struct Arena* arena) {
	return string_new(src.data, src.len, arena);
}

int string_blit_cstr(char* restrict dst, char* restrict src, isize max_len)
{
	isize charc = snprintf(dst, max_len, "%s", src);
	if (charc >= max_len) {
		ERROR("`src` contains more characters than allowed by `max_len`: %ld >= %ld", charc, max_len);
		return -1;
	}

	return 0;
}

int string_contains(String str, char c)
{
	for (int i = 0; i < str.len; i++)
		if (str.data[i] == c)
			return i;
	return -1;
}

bool string_endswith(String str, String ext)
{
	return !strncmp(str.data + str.len - ext.len, ext.data, ext.len);
}

bool string_strip_ext(String* str)
{
	char* p = str->data + str->len - 1;
	while (*p != '.') {
		if (p == str->data)
			return false;
		else
			p--;
	}

	*p = '\0';
	str->len = p - str->data;
	return true;
}

void string_clear(String* str)
{
	str->len     = 0;
	str->data[0] = '\0';
}

void string_cat_cstr(String* str1, char* str2, isize len)
{
	len = len > 0? len: (isize)strlen(str2);
	assert(str1->len + len < str1->cap);

	memcpy(str1->data + str1->len, str2, len*sizeof(char));
	str1->len += len;
	str1->data[str1->len] = '\0';
}

/* Remove character `c` from `str` and return the number of characters removed */
int string_remove(String str, char c)
{
	int i, j = 0;
	for (i = 0; i < str.len; i++)
		if (str.data[i] != c)
			str.data[j++] = str.data[i];

	int count = i - j;
	while (j < i)
		str.data[j++] = '\0';

	return count;
}

void string_free(String* str) {
	sfree(str->data);
	str->data = NULL;
	str->len = 0;
}

/* -------------------------------------------------------------------- */

char* string_file_name(String path)
{
	char* start = path.data + path.len;
	while (*--start != '/');
	return start + 1;
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
	while ((*ext++ = *++file));
}
