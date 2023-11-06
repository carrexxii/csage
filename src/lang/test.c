#ifdef TESTING_LANG

#include "lexer.h"
#include "repl.h"

int main(int argc, char** argv)
{
	struct TokenList* tokens = lexer_load("tests/test.lang");
	// lang_repl();

	sfree(tokens);
}

#endif /* TESTING_LANG */
