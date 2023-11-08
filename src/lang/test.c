#ifdef TESTING_LANG

#include "lexer.h"
#include "parser.h"
#include "repl.h"

int main(int argc, char** argv)
{
	struct TokenList* tokens = lexer_load("tests/test.lang");
	// for (int i = 0; i < tokens->tokenc; i++)
		// print_token(tokens->tokens[i]);
	struct AST ast = parser_parse(tokens);
	parser_print_ast(ast);

	// lang_repl();

	sfree(tokens);
}

#endif /* TESTING_LANG */
