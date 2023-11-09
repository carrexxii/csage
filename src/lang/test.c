#ifdef TESTING_LANG

#include "lexer.h"
#include "parser.h"
#include "bytecode.h"
#include "repl.h"

int main(int argc, char** argv)
{
	struct TokenList* tokens = lexer_load("tests/test.lang");
	// for (int i = 0; i < tokens->tokenc; i++)
		// print_token(tokens->tokens[i]);
	struct AST ast = parser_parse(tokens);
	DEBUG(1, " - - - - - - - - - - - - - - - - - - - ");
	parser_print_ast(ast);
	DEBUG(1, " - - - - - - - - - - - - - - - - - - - ");
	struct Bytecode code = bytecode_generate(ast);
	bytecode_print(code);
	DEBUG(1, " - - - - - - - - - - - - - - - - - - - ");

	// lang_repl();

	sfree(tokens);
}

#endif /* TESTING_LANG */
