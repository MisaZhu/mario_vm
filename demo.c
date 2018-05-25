#include "mario.h"
#include <stdio.h>

void dump(const char* s) {
	printf("%s", s);
}

var_t* native_print(vm_t* vm, var_t* env, void* data) {
	node_t* n = var_get(env, 0);
	if(n->var == NULL || n->var->value == NULL || n->var->type != V_STRING)
		return NULL;

	dump((const char*)n->var->value);
	return NULL;
}

int main(int argc, char** argv) {
	const char* s = "if(false) print('aaa\n');";
	while(true) {
		vm_t vm;
		vm_init(&vm);

		vm_reg_native(&vm, "print", 1, native_print);

		vm_load(&vm, s, dump);
		vm_run(&vm);
		vm_close(&vm);
	}
	return 0;
}
