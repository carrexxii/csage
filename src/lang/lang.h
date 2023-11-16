#ifndef LANG_LANG_H
#define LANG_LANG_H

#include "util/string.h"

enum LangType {
	LANG_INT = 1,
	LANG_FLT,
	LANG_STR,
	LANG_INT_LITERAL,
	LANG_TYPE_COUNT,
};

union LangVal {
	int64   s64;
	double  flt;
	String* str;
}; static_assert(sizeof(union LangVal) == 8, "union LangVal");

struct LangVar {
	enum LangType type;
	union LangVal val;
};

#endif
