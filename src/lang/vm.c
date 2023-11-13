#include "bytecode.h"
#include "util/htable.h"
#include "vm.h"

struct VM* vm_load(struct ByteCode code)
{
	// int litc_2 = 1u << (uint)log2(code.litc - 1) + 1u;

	struct VM* vm = smalloc(sizeof(struct VM) + VM_STACK_SIZE);
	vm->instrs = code.instrs;
	vm->lits   = code.lits;
	vm->vars   = smalloc(code.varc*sizeof(struct Literal));

	vm->ip  = 0;
	vm->sp  = 0;
	vm->acc = 0;

	DEBUG(4, "[LANG] Loaded bytecode into vm (%d literals, %d variables)", code.litc, code.varc);
	return vm;
}

void vm_run(struct VM* vm)
{
	struct Instruction instr = { .op = OP_NOOP };
	struct Literal lit;
	while (instr.op != OP_EOF) {
		instr = vm->instrs[vm->ip];
		switch (instr.op) {
		case OP_PUSH:
			assert(!instr.is_var && "Can only push literals rn");
			lit = vm->lits[instr.operand];
			switch (lit.type) {
			case LIT_INT : vm->stack[vm->sp] = lit.integer; break;
			case LIT_REAL: vm->stack[vm->sp] = lit.real;    break;
			case LIT_STR : vm->stack[vm->sp] = (intptr)&lit.string; break; // !
			default:
				assert(false && "Invalid type for literal");
			}
			vm->sp++;
			DEBUG(1, "Pushed %ld", vm->stack[vm->sp - 1]);
			break;
		case OP_POP:
			assert(instr.is_var && "Must pop into variable");
			lit = vm->lits[instr.operand];
			vm->sp--;
			switch (lit.type) {
			case LIT_INT : vm->vars[instr.operand].integer = vm->stack[vm->sp]; break;
			case LIT_REAL: vm->vars[instr.operand].real    = vm->stack[vm->sp]; break;
			case LIT_STR : vm->vars[instr.operand].string  = STRING("Not implemented"); break; // !
			default:
				assert(false && "Invalid type for literal");
			}
			DEBUG(1, "Popped %ld into %d", vm->stack[vm->sp], instr.operand);
			break;
		case OP_CALL:
			DEBUG_VALUE(vm->vars[instr.operand].string.data);
			DEBUG_VALUE(instr.operand);
			if (instr.operand == 0xFF)
				printf(TERM_MAGENTA "\t[%ld]\n" TERM_NORMAL, vm->stack[--vm->sp]);
			break;
		case OP_EOF:
			break;
		default:
			// ERROR("[LANG] Unrecognized instruction: %s", STRING_OF_OP(instr.op));
			ERROR("[LANG] Unrecognized instruction: %d", instr.op);
		}
		vm->ip++;
	}
}

void vm_free(struct VM* vm)
{
	sfree(vm->vars);
	// sfree(vm->lits);
	sfree(vm);
}
