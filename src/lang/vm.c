#include "util/htable.h"
#include "vm.h"

struct VM* vm_load(struct ByteCode code)
{
	int litc_2 = 1u << (uint)log2(code.litc - 1) + 1u;

	struct VM* vm = smalloc(sizeof(struct VM) + VM_STACK_SIZE);
	vm->lits = smalloc(litc_2);
	vm->vars = htable_new(128); // TODO: bytecode enerator should count the number of variables for this

	vm->ip  = 0;
	vm->sp  = 0;
	vm->acc = 0;

	DEBUG(4, "[LANG] Loaded bytecode into vm (%d literals, %d variables)", litc_2, code.varc);
	return vm;
}

void vm_run(struct VM* vm)
{

}

void vm_free(struct VM* vm)
{
	htable_free(vm->vars);
	sfree(vm->lits);
	sfree(vm);
}
