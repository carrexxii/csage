#ifndef LANG_LEXER_H
#define LANG_LEXER_H

#include "util/string.h"

enum TokenType {
	TOKEN_NONE,
	TOKEN_EOF,
	TOKEN_IDENT,
	TOKEN_NUMBER,
	TOKEN_STRING,
	TOKEN_SYMBOL,

	TOKEN_VAL,
	TOKEN_VAR,
	TOKEN_LET,
	TOKEN_IN,
	TOKEN_FUN,
	TOKEN_IF,
	TOKEN_THEN,
	TOKEN_ELSE,
	TOKEN_OF,
};

struct Token {
	String lexeme;
	enum TokenType type;
	int line;
	int col;
};

struct TokenList {
	intptr tokenc;
	struct Token tokens[];
};

struct TokenList* lexer_tokenize(char* restrict text);
struct TokenList* lexer_load(char* restrict fname);
void print_token(struct Token token);

#define STRING_OF_TOKEN(_e0) \
	(_e0) == TOKEN_NONE?   "TOKEN_NONE":   (_e0) == TOKEN_EOF?    "TOKEN_EOF":    (_e0) == TOKEN_IDENT?  "TOKEN_IDENT":  \
	(_e0) == TOKEN_NUMBER? "TOKEN_NUMBER": (_e0) == TOKEN_STRING? "TOKEN_STRING": (_e0) == TOKEN_SYMBOL? "TOKEN_SYMBOL": \
	"<Unknown value for enum \"TokenType\">"
#endif
