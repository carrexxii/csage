#ifndef LANG_VM_H
#define LANG_VM_H

#include "util/varray.h"
#include "util/htable.h"
#include "bytecode.h"

#define VM_STACK_SIZE 4*1024

struct VM {
	struct Instruction* instrs;
	// TODO: copy the bytecode's hashtable for variables for the repl?
	struct VArray vars;
	struct VArray lits;
	intptr entry;
};

struct VM vm_load(struct ByteCode* code);
void vm_run(struct VM vm);

#endif
