#include "util/htable.h"
#include "util/varray.h"
#include "lang.h"
#include "bytecode.h"
#include "vm.h"

noreturn static void error_unknown_instr(struct Instruction instr);

struct VM vm_load(struct ByteCode code)
{
	// int litc_2 = 1u << (uint)log2(code.litc - 1) + 1u;

	struct VM vm;
	// TODO: improve/copy
	vm.instrs = (struct Instruction*)code.instrs->data;
	vm.lits   = (union LangVal*)code.lits->data;
	vm.vars   = smalloc(code.vars->len*sizeof(union LangVal));

	DEBUG(4, "[LANG] Loaded bytecode into vm (%ld literals, %ld variables)", code.lits->len, code.vars->len);
	return vm;
}

#define next() do {                       \
		DEBUG_VALUE(ip);                   \
		ci = vm.instrs[ip];                 \
		goto *op_labels[vm.instrs[ip++].op]; \
	} while (0)
void vm_run(struct VM vm)
{
	static void* op_labels[] = {
		[OP_NOOP] = &&op_noop,
		[OP_PUSH] = &&op_push,
		[OP_POP]  = &&op_pop,
		[OP_CALL] = &&op_call,
		[OP_RET]  = &&op_ret,
		[OP_EOF]  = &&op_eof,
	};

	union LangVal* stack = smalloc(VM_STACK_SIZE);
	register intptr sp = 0;
	register intptr ip = 0;
	register struct Instruction ci;
	next();

op_noop:
	next();

op_push:
	if (ci.type == LANG_INT_LITERAL)
		stack[sp].s64 = ci.operand;
	else
		stack[sp] = vm.lits[ci.operand];
	sp++;
	next();

op_pop:
	vm.vars[ci.operand] = stack[--sp];
	next();

op_call:
	if (ci.operand == 0xFF)
		printf("%ld\n", stack[--sp].s64);
	stack[sp++] = (union LangVal){ .s64 = ip };
	ip = ci.operand;
	next();

op_ret:
	ip = stack[--sp].s64;
	next();

op_eof:
	sfree(stack);
}
#undef next

void vm_free(struct VM vm)
{
	sfree(vm.vars);
	// sfree(vm.lits);
}
