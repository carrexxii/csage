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
	// TODO: add tagged union array for constants/literals
	struct HTable*  vars;
	struct Literal* lits;

	/* Registers */
	intptr sp;
	intptr ip;
	intptr acc;
	enum VMFlag flag;

	byte stack[];
};

struct VM* vm_load(struct ByteCode code);
void vm_run(struct VM* vm);
void vm_free(struct VM* vm);

#endif
