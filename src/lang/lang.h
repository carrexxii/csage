#ifndef LANG_LANG_H
#define LANG_LANG_H

#include "util/string.h"

enum LangType {
	LANG_INT,
	LANG_FLT,
	LANG_STR,
	LANG_INT_LITERAL,
	LANG_TYPE_COUNT,
};

union LangVal {
	int64  s64;
	double dbl;
	char*  str;
}; static_assert(sizeof(union LangVal) == 8, "union LangVal");

// struct LangVal {
// 	enum LangType     type;
// 	union LangTypeVal val;
// };

#endif
