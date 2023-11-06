#ifndef LANG_LEXER_H
#define LANG_LEXER_H

enum TokenType {
	TOKEN_NONE,
	TOKEN_EOF,

	TOKEN_IDENT,
	TOKEN_VAR,
	TOKEN_LET,

	TOKEN_NUMBER,
	TOKEN_STRING,

	TOKEN_EQ,
};

struct Token {
	char* lexeme;
	enum TokenType type;
	int line;
};

struct TokenList {
	intptr tokenc;
	struct Token tokens[];
};

struct TokenList* lexer_tokenize(char* text);
struct TokenList* lexer_load(char* fname);
void print_token(struct Token token);

#define STRING_OF_TOKEN(t) \
	t == TOKEN_NONE  ? "TOKEN_NONE"  : t == TOKEN_VAR   ? "TOKEN_VAR"   : t == TOKEN_LET  ? "TOKEN_LET"  : \
	t == TOKEN_EQ    ? "TOKEN_EQ"    : t == TOKEN_EOF   ? "TOKEN_EOF"   : t == TOKEN_IDENT? "TOKEN_IDENT": \
	t == TOKEN_NUMBER? "TOKEN_NUMBER": t == TOKEN_STRING? "TOKEN_STRING": \
	"<Unknown Token>"

#endif
