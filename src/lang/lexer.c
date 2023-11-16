#include "util/file.h"
#include "util/string.h"
#include "lexer.h"

#define NEXT() EXPR(         \
		tknz->col++;          \
		c = fgetc(tknz->file); \
	)
#define BACK(c) EXPR(        \
		tknz->col--;          \
		ungetc(c, tknz->file); \
	)
#define TOKEN(t, l, len) (struct Token){ \
		.type   = t,                     \
		.lexeme = l,                     \
		.line   = tknz->line,            \
		.col    = tknz->col - len        \
	}
#define LEXER_BUFFER_SIZE      1024
#define LEXER_TOKEN_MULTIPLIER 2
#define LEXER_SYMBOLS          "-=_+[]{};':\"\\|,.<>/?`~!@#$%^&*()"

noreturn static void error_unhandled(char c);

inline static char skip_whitespace(struct Tokenizer* tknz, char c);
inline static char skip_comment(struct Tokenizer* tknz, char c);

inline static struct Token read_number(struct Tokenizer* tknz, char c);
inline static struct Token read_string(struct Tokenizer* tknz, char c);
inline static struct Token read_ident(struct Tokenizer* tknz, char c);
inline static struct Token read_keyword(struct Tokenizer* tknz, char c);
inline static struct Token read_symbol(struct Tokenizer* tknz, char c);
inline static bool can_match(char c1, char c2);

struct Tokenizer lexer_load_file(char* fname)
{
	String64 path;
	snprintf(path.data, sizeof(path), SCRIPT_PATH "%s", fname);
	struct Tokenizer tokenizer = {
		.file      = file_open(path.data, "r"),
		.file_name = string_new_split(path.data, '/', -1),
		.line      = 1,
		.col       = 1,
	};

	DEBUG(2, "[LANG] Created new tokenizer from file \"%s\"", fname);
	return tokenizer;
}

struct Token lexer_next(struct Tokenizer* tknz)
{
	char c;
	NEXT();
	while (1) {
		if (isspace(c))
			c = skip_whitespace(tknz, c);
		if (c == '(')
			c = skip_comment(tknz, c);

		if (isdigit(c))
			return read_number(tknz, c);
		else if (c == '"')
			return read_string(tknz, c);
		else if (isalpha(c) || c == '_')
			return read_ident(tknz, c);
		else if (isgraph(c))
			return read_symbol(tknz, c);
		else if (c == EOF) {
			fclose(tknz->file);
			return TOKEN(TOKEN_EOF, (String){ 0 }, 0);
		}
	}
}

void print_token(struct Token token)
{
	fprintf(stderr, "%s \t %s on line %d, col %d\n", token.lexeme.data, STRING_OF_TOKEN(token.type), token.line, token.col);
}

/* -------------------------------------------------------------------- */

inline static char skip_whitespace(struct Tokenizer* tknz, char c)
{
	while (isspace(c)) {
		if (c == '\n') {
			tknz->line++;
			tknz->col = 1;
		}
		NEXT();
	}

	return c;
}

inline static char skip_comment(struct Tokenizer* tknz, char c)
{
	if (c == '(' && NEXT() == '*')
		while (1)
			if (NEXT() == '*' && NEXT() == ')')
				return NEXT();

	BACK(c);
	return '(';
}

inline static struct Token read_number(struct Tokenizer* tknz, char c)
{
	char buf[64];
	int len = 0;
	bool dp = false;
	while (isdigit(c) || c == '.') {
		if (c == '.')
			if (!(dp = !dp))
				ERROR("[LANG] Multiple decimal places in number");
		buf[len++] = c;
		NEXT();
	}

	BACK(c);
	return TOKEN(TOKEN_NUMBER, string_new(buf, len), len);
}

inline static struct Token read_string(struct Tokenizer* tknz, char c)
{
	char buf[LEXER_MAX_STRING_LEN];
	int len = 0;
	NEXT();
	while (c != '"') {
		if (c == '\n') {
			ERROR("[LANG] Expected end of string before end of line");
			exit(1);
		}
		buf[len++] = c;
		NEXT();
	}

	return TOKEN(TOKEN_STRING, string_new(buf, len), len);
}

#define CHECK(t, l)                                             \
	if (len == sizeof(l) - 1 && !strncmp(buf, l, sizeof(l) - 1)) \
		return TOKEN(t, STRING(l), len);
inline static struct Token read_ident(struct Tokenizer* tknz, char c)
{
	char buf[LEXER_MAX_IDENT_LEN];
	int len = 0;
	while (isalnum(c) || c == '_') {
		buf[len++] = c;
		NEXT();
	}

	BACK(c);
	CHECK(TOKEN_VAL , "val");
	CHECK(TOKEN_VAR , "var");
	CHECK(TOKEN_LET , "let");
	CHECK(TOKEN_IN  , "in");
	CHECK(TOKEN_FUN , "fun");
	CHECK(TOKEN_IF  , "if");
	CHECK(TOKEN_THEN, "then");
	CHECK(TOKEN_ELSE, "else");
	CHECK(TOKEN_OF  , "of");
	return TOKEN(TOKEN_IDENT, string_new(buf, len), len);
}

inline static struct Token read_symbol(struct Tokenizer* tknz, char c)
{
	char buf[8];
	int len = 0;
	do {
		if (isspace(c) || isalnum(c)) {
			BACK(c);
			break;
		} else if (len && !can_match(c, buf[len - 1])) {
			BACK(c);
			break;
		}

		buf[len++] = c;
		NEXT();
	} while (1);

	return TOKEN(TOKEN_SYMBOL, string_new(buf, len), len);
}

inline static bool can_match(char c1, char c2) {
	return (c1 == ')' && c2 == '(');
}

/* -------------------------------------------------------------------- */

noreturn static void error_unhandled(char c)
{
	ERROR("[LANG] Error handling character: \"%c\" (%d)", c, c);
	exit(1);
}
