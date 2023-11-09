#ifndef LANG_BYTECODE_H
#define LANG_BYTECODE_H

#include "parser.h"

enum BytecodeOp {
	OP_NOOP,
	OP_PUSH,
	OP_POP,
	OP_CODE_MAX,
};

void bytecode_generate(struct AST ast);

#endif
