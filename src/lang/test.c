#ifdef TESTING_LANG

#include "lexer.h"
#include "parser.h"
#include "bytecode.h"
#include "vm.h"
#include "repl.h"

int main(int argc, char** argv)
{
	// struct TokenList* tokens = lexer_load("tests/test.lang");
	// for (int i = 0; i < tokens->tokenc; i++)
		// print_token(tokens->tokens[i]);
	struct Tokenizer tknz = lexer_load_file("tests/test.lang");
	// struct Token tok;
	// while ((tok = lexer_next(&tknz)).type != TOKEN_EOF)
	// 	print_token(tok);
	// print_token(tok);
	DEBUG(1, " - - - - - - - - - Parser - - - - - - - - - - - ");
	struct AST ast = parser_parse(tknz);
	DEBUG(1, " - - - - - - - - - AST - - - - - - - - - - - - ");
	parser_print_ast(ast);
	DEBUG(1, " - - - - - - - - - ByteCode - - - - - - - - - - ");
	struct ByteCode code = bytecode_generate(&ast);
	bytecode_print(code);
	DEBUG(1, " - - - - - - - - - VM - - - - - - - - - - ");
	struct VM vm = vm_load(&code);
	vm_run(vm);
	DEBUG(1, " - - - - - - - - - - - - - - - - - - - ");

	// lang_repl();
}

#endif /* TESTING_LANG */
