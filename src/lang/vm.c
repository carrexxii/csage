#include "util/htable.h"
#include "util/varray.h"
#include "lang.h"
#include "bytecode.h"
#include "vm.h"

noreturn static void error_unknown_instr(struct Instruction instr);

struct VM vm_load(struct ByteCode* code)
{
	// int litc_2 = 1u << (uint)log2(code.litc - 1) + 1u;

	struct VM vm;
	// TODO: improve/copy
	vm.instrs = (struct Instruction*)code->instrs.data;
	vm.lits   = code->lits;
	vm.vars   = code->vars;
	vm.entry  = code->entry;

	DEBUG(4, "[LANG] Loaded %d bytecode instructions into vm (%d literals, %d variables)\n\tEntry point: %ld",
	      code->instrs.len, code->lits.len, code->vars.len, code->entry);
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
	static_assert(OP_CODE_MAX == 6, "`op_labels` needs to be updated");

	int64* stack = smalloc(VM_STACK_SIZE);
	register intptr sp = 0;
	register intptr fp = 0;
	register intptr ip = vm.entry;
	register struct Instruction ci;

	next();

op_noop:
	next();

op_push:
	if (ci.type == LANG_INT_LITERAL)
		stack[sp] = ci.operand;
	// else
		// stack[sp] = varray_get(vm.lits, ci.operand);
	sp++;
	next();

op_pop:
	varray_set(&vm.vars, ci.operand, &stack[--sp]);
	next();

/* Calling:
 * - Push the current instruction onto the stack (for returning)
 * - Save the stack pointer into the frame pointer (Gets reset when we return)
 * - Set the instruction pointer to the called address
 */
op_call:
	stack[sp++] = ip;
	fp = sp;

	ip = *(int64*)varray_get(&vm.vars, ci.operand);
	next();

/* Returning:
 * - Reset the stack pointer back to the start of the stack frame
 * - Get the return address from the top of the stack
 */
op_ret:
	sp = fp - 1;
	ip = stack[sp];
	next();

op_eof:
	sfree(stack);
}
