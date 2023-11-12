#ifndef LANG_VM_H
#define LANG_VM_H

#include "util/htable.h"
#include "bytecode.h"

#define VM_STACK_SIZE 4*1024

enum VMFlag {
	FLAG_LT = -1,
	FLAG_GT =  0,
	FLAG_EQ =  1,
};

struct VM {
	struct Instruction* instrs;
	// TODO: copy the bytecode's hashtable for variables for the repl?
	struct Literal* vars;
	struct Literal* lits;

	/* Registers */
	intptr sp;
	intptr ip;
	intptr acc;
	enum VMFlag flag;

	int64 stack[]; // TODO: better type
};

struct VM* vm_load(struct ByteCode code);
void vm_run(struct VM* vm);
void vm_free(struct VM* vm);

#endif
