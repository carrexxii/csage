#ifndef LANG_LANG_H
#define LANG_LANG_H

#include "util/string.h"

enum LangType {
	LANG_NONE,
	LANG_INT,
	LANG_FLT,
	LANG_STR,
	LANG_BOOL,
	LANG_INT_LITERAL,
	LANG_VAR,
	LANG_FUN,
	LANG_TYPE_COUNT,
};

union LangVal {
	int64   s64;
	double  flt;
	String* str;
}; static_assert(sizeof(union LangVal) == 8, "union LangVal");

struct LangVar {
	enum LangType  type;
	union LangVal  val;
	struct VArray* params;
};

#define STRING_OF_TYPE(_e0) \
	(_e0) == LANG_INT?         "LANG_INT":         (_e0) == LANG_FLT?  "LANG_FLT":  \
	(_e0) == LANG_STR?         "LANG_STR":         (_e0) == LANG_BOOL? "LANG_BOOL": \
	(_e0) == LANG_INT_LITERAL? "LANG_INT_LITERAL": (_e0) == LANG_FUN?  "LANG_FUN":  \
	(_e0) == LANG_VAR?         "LANG_VAR":         (_e0) == LANG_NONE? "LANG_NONE": \
	"<Unknown value for enum \"LangType\">"


#endif
