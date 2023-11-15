#include "util/file.h"
#include "util/string.h"
#include "lexer.h"

#define LEXER_BUFFER_SIZE      1024
#define LEXER_TOKEN_MULTIPLIER 2
#define LEXER_SYMBOLS          "-=_+[]{};':\"\\|,.<>/?`~!@#$%^&*()"

noreturn static void error_unhandled(char c);

inline static int read_number(char* text);
inline static int read_string(char* text);
inline static int read_ident(char* text);
inline static int read_keyword(char* text, enum TokenType* type);
inline static int read_symbol(char* text);

struct Tokenizer lexer_load_file(char* path)
{
	// struct Tokenizer tokenizer;
	// tokenizer.file = file_open(path);
	// tokenizer.file_name = string_new_split(path, '/', -1);
	// tokenizer.cursor = NULL;
}

struct TokenList* lexer_tokenize(char* text)
{
	intptr max_tokens = 1024;
	struct TokenList* tokens = smalloc(sizeof(struct TokenList) + max_tokens*sizeof(struct Token));
	tokens->tokenc = 0;

	struct Token* token;
	char* line_start = text;
	int line = 1;
	int len;
	char c;
	do {
		if (tokens->tokenc >= max_tokens) {
			max_tokens *= LEXER_TOKEN_MULTIPLIER;
			tokens = srealloc(tokens, sizeof(struct TokenList) + max_tokens*sizeof(struct Token));
		}

		c = *text;
		len = 1;
		if (isspace(c)) {
			text++;
			if (c == '\n') {
				line++;
				line_start = text;
			}
			continue;
		}

		token = &tokens->tokens[tokens->tokenc++];
		if (c == '(' &&  *(text + 1) == '*') {
			text += 2;
			while (*text != '*' &&  *(text + 1) != ')')
				text++;
			text += 2;
			len = 0;
			tokens->tokenc--;
		} else if (isdigit(c)) {
			len = read_number(text);
			token->type = TOKEN_NUMBER;
		} else if (c == '"') {
			len = read_string(text);
			token->type = TOKEN_STRING;
		} else if (isalpha(c) || c == '_') {
			len = read_keyword(text, &token->type);
			if (token->type == TOKEN_NONE) {
				len = read_ident(text);
				token->type = TOKEN_IDENT;
			}
		} else if (isgraph(c)) {
			len = read_symbol(text);
			token->type = TOKEN_SYMBOL;
		} else {
			error_unhandled(*text);
		}

		// TODO: cleanup
		if (len == -1) {
			ERROR("[LANG] Line: %d", line);
			return NULL;
		}

		if (len) { /* In case of a comment */
			token->lexeme = string_new(text, len);
			token->line   = line;
			token->col    = text - line_start + 1;
			text += len;
		}
	} while (*text);

	tokens->tokens[tokens->tokenc++] = (struct Token){
		.type = TOKEN_EOF,
		.line = line,
	};

	return tokens;
}

struct TokenList* lexer_load(char* fname)
{
	String64 path;
	snprintf(path.data, sizeof(path), SCRIPT_PATH "%s", fname);
	char* text = file_load(path.data);
	struct TokenList* tokens = lexer_tokenize(text);

	sfree(text);
	return tokens;
}

void print_token(struct Token token)
{
	fprintf(stderr, "%s \t %s on line %d, col %d\n", token.lexeme.data, STRING_OF_TOKEN(token.type), token.line, token.col);
}

inline static int read_number(char* text)
{
	int len = 0;
	bool dp = false;
	while (isdigit(*text) || (*text == '.' && !dp)) {
		if (*text == '.')
			dp = true;
		len++;
		text++;
	}

	return len;
}

inline static int read_string(char* text)
{
	int len = 1;
	text++;
	while (*text != '"') {
		if (*text == '\n') {
			ERROR("Expected end of string before end of line");
			return -1;
		}
		len++;
		text++;
	}

	return len + 1;
}

inline static int read_ident(char* text)
{
	int len = 1;
	text++;
	while (isalnum(*text) || *text == '_') {
		len++;
		text++;
	}

	return len;
}

#define check(_v, _t)                        \
	if (!strncmp(text, _v, sizeof(_v) - 1)) { \
		*type = _t;                            \
		return sizeof(_v) - 1;                  \
	}
inline static int read_keyword(char* text, enum TokenType* type)
{
	check("val" , TOKEN_VAL);
	check("var" , TOKEN_VAR);
	check("let" , TOKEN_LET);
	check("in"  , TOKEN_IN);
	check("fun" , TOKEN_FUN);
	check("if"  , TOKEN_IF);
	check("then", TOKEN_THEN);
	check("else", TOKEN_ELSE);
	check("of"  , TOKEN_OF);

	*type = TOKEN_NONE;
	return 0;
}
#undef check

inline static int read_symbol(char* text)
{
	int len = 0;
	do {
		if (isspace(*text) || isalnum(*text))
            break;
		len++;
		text++;
	} while (1);

	return len;
}

noreturn static void error_unhandled(char c)
{
	ERROR("[LANG] Error handling character: \"%c\"", c);
	exit(1);
}
