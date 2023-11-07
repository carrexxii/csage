#ifdef TESTING_LANG

#include "lexer.h"
#include "repl.h"

int main(int argc, char** argv)
{
	struct TokenList* tokens = lexer_load("tests/test.lang");
	if (!tokens)
		exit(1);
	for (int i = 0; i < tokens->tokenc; i++)
		print_token(tokens->tokens[i]);
	// lang_repl();

	sfree(tokens);
}

#endif /* TESTING_LANG */
