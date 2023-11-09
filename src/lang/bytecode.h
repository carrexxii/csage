#ifndef LANG_BYTECODE_H
#define LANG_BYTECODE_H

#include "parser.h"

enum BytecodeOp {
	OP_NOOP,
	OP_PUSH,
	OP_POP,
	OP_CODE_MAX,
};

struct Bytecode {
	byte*  data;
	byte*  sp;
	intptr capacity;
};

struct Bytecode bytecode_generate(struct AST ast);
void bytecode_print(struct Bytecode code);

#endif
