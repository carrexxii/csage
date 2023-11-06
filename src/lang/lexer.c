#include "util/file.h"
#include "util/strings.h"
#include "lexer.h"

#define LEXER_BUFFER_SIZE      1024
#define LEXER_TOKEN_MULTIPLIER 2
#define LEXER_SYMBOLS          "-=_+[]{};':\"\\|,.<>/?`~!@#$%^&*()"

inline static bool is_number(char* str, int len);
inline static bool is_string(char* str, int len);

struct TokenList* lexer_tokenize(char* text)
{
	intptr max_tokens = 128;
	struct TokenList* tokens = smalloc(sizeof(struct TokenList) + max_tokens*sizeof(struct Token));
	tokens->tokenc = 0;

	int buf_len;
	char buf[LEXER_BUFFER_SIZE];
	enum TokenType type;
	int line = 1;
	do {
		buf_len = 0;
		while (isspace(*text)) {
			if (*text == '\n')
				line++;
			text++;
		}

		while (*text && !isspace(*text))
			buf[buf_len++] = *text++;
		buf[buf_len] = '\0';

		if (buf_len <= 0) {
			continue;
		} else if (is_number(buf, buf_len)) {
			type = TOKEN_NUMBER;
		} else if (is_string(buf, buf_len)) {
			type = TOKEN_STRING;
		} else if (isgraph(*buf)) {
			#define is_token(s) !strncmp(buf, s, LEXER_BUFFER_SIZE)
			type = is_token("var")? TOKEN_VAR:
			       is_token("let")? TOKEN_LET:
			       is_token("=")  ? TOKEN_EQ :
			       is_token("")   ? TOKEN_NONE:
			       TOKEN_IDENT;
			#undef is_token
		} else {
			type = TOKEN_NONE;
		}

		if (tokens->tokenc >= max_tokens) {
			max_tokens *= LEXER_TOKEN_MULTIPLIER;
			tokens = srealloc(tokens, sizeof(struct TokenList) + max_tokens*sizeof(struct Token));
		}
		tokens->tokens[tokens->tokenc] = (struct Token){
			.type = type,
			.line = line,
		};
		tokens->tokens[tokens->tokenc].lexeme = string_new(buf, buf_len);
		print_token(tokens->tokens[tokens->tokenc]);
		tokens->tokenc++;
	} while (*text);

	return tokens;
}

struct TokenList* lexer_load(char* fname)
{
	ShortString path;
	snprintf(path, SHORTSTRING_LENGTH, SCRIPT_PATH "%s", fname);
	char* text = file_load(path);
	struct TokenList* tokens = lexer_tokenize(text);

	sfree(text);
	return tokens;
}

void print_token(struct Token token)
{
	fprintf(stderr, "%s \t %s on line %d\n", token.lexeme, STRING_OF_TOKEN(token.type), token.line);
}

inline static bool is_number(char* str, int len)
{
	if (len <= 0)
		return false;

	for (int i = 0; i < len; i++)
		if (!isdigit(str[i]))
			return false;

	return true;
}

inline static bool is_string(char* str, int len)
{
	if (len <= 0)
		return false;

	return str[0]       == '"' &&
	       str[len - 1] == '"';
}
