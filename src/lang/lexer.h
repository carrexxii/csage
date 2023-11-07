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

	TOKEN_SYMBOL, // !
	TOKEN_EQ,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_STAR,
	TOKEN_FSLASH,
	TOKEN_BSLASH,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACKET,
	TOKEN_RBRACKET,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_TILDE,
	TOKEN_BTICK,
	TOKEN_AT,
	TOKEN_HASH,
	TOKEN_DOLLAR,
	TOKEN_PERCENT,
	TOKEN_CARET,
	TOKEN_AMPER,
	TOKEN_UNDERSCORE,
	TOKEN_COLON,
	TOKEN_SCOLON,
	TOKEN_SQUOTE,
	TOKEN_DQUOTE,
	TOKEN_PERIOD,
	TOKEN_COMMA,
	TOKEN_LT,
	TOKEN_GT,
	TOKEN_PIPE,
	TOKEN_EMARK,
	TOKEN_QMARK,
};

struct Token {
	char* lexeme;
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
	(_e0) == TOKEN_NONE?     "TOKEN_NONE":     (_e0) == TOKEN_EOF?        "TOKEN_EOF":        \
	(_e0) == TOKEN_IDENT?    "TOKEN_IDENT":    (_e0) == TOKEN_VAR?        "TOKEN_VAR":        \
	(_e0) == TOKEN_LET?      "TOKEN_LET":      (_e0) == TOKEN_NUMBER?     "TOKEN_NUMBER":     \
	(_e0) == TOKEN_STRING?   "TOKEN_STRING":   (_e0) == TOKEN_EQ?         "TOKEN_EQ":         \
	(_e0) == TOKEN_PLUS?     "TOKEN_PLUS":     (_e0) == TOKEN_MINUS?      "TOKEN_MINUS":      \
	(_e0) == TOKEN_STAR?     "TOKEN_STAR":     (_e0) == TOKEN_FSLASH?     "TOKEN_FSLASH":     \
	(_e0) == TOKEN_BSLASH?   "TOKEN_BSLASH":   (_e0) == TOKEN_LPAREN?     "TOKEN_LPAREN":     \
	(_e0) == TOKEN_RPAREN?   "TOKEN_RPAREN":   (_e0) == TOKEN_LBRACKET?   "TOKEN_LBRACKET":   \
	(_e0) == TOKEN_RBRACKET? "TOKEN_RBRACKET": (_e0) == TOKEN_LBRACE?     "TOKEN_LBRACE":     \
	(_e0) == TOKEN_RBRACE?   "TOKEN_RBRACE":   (_e0) == TOKEN_TILDE?      "TOKEN_TILDE":      \
	(_e0) == TOKEN_BTICK?    "TOKEN_BTICK":    (_e0) == TOKEN_AT?         "TOKEN_AT":         \
	(_e0) == TOKEN_HASH?     "TOKEN_HASH":     (_e0) == TOKEN_DOLLAR?     "TOKEN_DOLLAR":     \
	(_e0) == TOKEN_PERCENT?  "TOKEN_PERCENT":  (_e0) == TOKEN_CARET?      "TOKEN_CARET":      \
	(_e0) == TOKEN_AMPER?    "TOKEN_AMPER":    (_e0) == TOKEN_UNDERSCORE? "TOKEN_UNDERSCORE": \
	(_e0) == TOKEN_COLON?    "TOKEN_COLON":    (_e0) == TOKEN_SCOLON?     "TOKEN_SCOLON":     \
	(_e0) == TOKEN_SQUOTE?   "TOKEN_SQUOTE":   (_e0) == TOKEN_DQUOTE?     "TOKEN_DQUOTE":     \
	(_e0) == TOKEN_PERIOD?   "TOKEN_PERIOD":   (_e0) == TOKEN_COMMA?      "TOKEN_COMMA":      \
	(_e0) == TOKEN_LT?       "TOKEN_LT":       (_e0) == TOKEN_GT?         "TOKEN_GT":         \
	(_e0) == TOKEN_PIPE?     "TOKEN_PIPE":     (_e0) == TOKEN_EMARK?      "TOKEN_EMARK":      \
	(_e0) == TOKEN_QMARK?    "TOKEN_QMARK":    "<Unknown value for enum \"TokenType\">"

#endif
