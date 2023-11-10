#include "util/file.h"
#include "util/string.h"
#include "lexer.h"

#define LEXER_BUFFER_SIZE      1024
#define LEXER_TOKEN_MULTIPLIER 2
#define LEXER_SYMBOLS          "-=_+[]{};':\"\\|,.<>/?`~!@#$%^&*()"

inline static int read_number(char* text);
inline static int read_string(char* text);
inline static int read_ident(char* text);
inline static enum TokenType read_symbol(char* text);

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
		if (isdigit(c)) {
			len = read_number(text);
			token->type = TOKEN_NUMBER;
		} else if (c == '"') {
			len = read_string(text);
			token->type = TOKEN_STRING;
		} else if (isalpha(c) || c == '_') {
			len = read_ident(text);
			token->type = TOKEN_IDENT;
		} else if (isgraph(c)) {
			len = 1;
			token->type = read_symbol(text);
		} else {
			ERROR("[LANG] Unexpected character: %c", *text);
		}

		// TODO: cleanup
		if (len == -1) {
			ERROR("[LANG] Line: %d", line);
			return NULL;
		}

		token->lexeme = string_new(text, len);
		token->line   = line;
		token->col    = text - line_start + 1;
		text += len;
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
	fprintf(stderr, "%s \t %s on line %d, col %d\n", token.lexeme->data, STRING_OF_TOKEN(token.type), token.line, token.col);
}

inline static int read_number(char* text)
{
	int len = 0;
	while (isdigit(*text)) {
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

inline static enum TokenType read_symbol(char* text)
{
	switch (*text) {
		case '=' : return TOKEN_EQ;
		case '+' : return TOKEN_PLUS;
		case '-' : return TOKEN_MINUS;
		case '*' : return TOKEN_STAR;
		case '/' : return TOKEN_FSLASH;
		case '\\': return TOKEN_BSLASH;
		case '(' : return TOKEN_LPAREN;
		case ')' : return TOKEN_RPAREN;
		case '[' : return TOKEN_LBRACKET;
		case ']' : return TOKEN_RBRACKET;
		case '{' : return TOKEN_LBRACE;
		case '}' : return TOKEN_RBRACE;
		case '~' : return TOKEN_TILDE;
		case '`' : return TOKEN_BTICK;
		case '@' : return TOKEN_AT;
		case '#' : return TOKEN_HASH;
		case '$' : return TOKEN_DOLLAR;
		case '%' : return TOKEN_PERCENT;
		case '^' : return TOKEN_CARET;
		case '&' : return TOKEN_AMPER;
		case '_' : return TOKEN_UNDERSCORE;
		case ':' : return TOKEN_COLON;
		case ';' : return TOKEN_SCOLON;
		case '\'': return TOKEN_SQUOTE;
		case '"' : return TOKEN_DQUOTE;
		case '.' : return TOKEN_PERIOD;
		case ',' : return TOKEN_COMMA;
		case '<' : return TOKEN_LT;
		case '>' : return TOKEN_GT;
		case '|' : return TOKEN_PIPE;
		case '!' : return TOKEN_EMARK;
		case '?' : return TOKEN_QMARK;
		default:
			ERROR("[LANG] Unrecognized symbol: %c", *text);
			return TOKEN_NONE;
	}
}
