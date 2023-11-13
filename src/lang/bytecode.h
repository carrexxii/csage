#ifndef LANG_BYTECODE_H
#define LANG_BYTECODE_H

#include "util/htable.h"
#include "lang.h"
#include "parser.h"

#define BYTECODE_DEFAULT_SIZE    128
#define BYTECODE_LITERAL_COUNT   16
#define BYTECODE_SIZE_MULTIPLIER 2
#define BYTECODE_HTABLE_SIZE     32

enum ByteCodeOp {
	OP_NOOP,
	OP_PUSH,
	OP_POP,
	OP_CALL,
	OP_EOF,
	OP_CODE_MAX,
};

enum LiteralType {
	LIT_NONE,
	LIT_INT  = AST_INT,
	LIT_REAL = AST_REAL,
	LIT_STR  = AST_STR,
};

/* Operand will be a reference to either a literal in the literals table or a variable in the variables table */
struct Instruction {
	enum ByteCodeOp op: 8;
	enum LangType type: 8;
	int16 operand;
}; static_assert(sizeof(struct Instruction) == 4, "struct Instruction");

struct ByteCode {
	struct Instruction* instrs;
	int instr_cap;
	int instrc;
	union LangVal* lits;
	int lit_cap;
	int litc;
	int varc;
	struct HTable* lit_table;
	struct HTable* var_table;
};

struct ByteCode bytecode_generate(struct AST ast);
void bytecode_print(struct ByteCode code);
void bytecode_free(struct ByteCode code);

#endif
