#include "mario.h"
#include <stdio.h>

void dump(const char* s) {
	printf("%s", s);
}

var_t* native_print(vm_t* vm, var_t* env, void* data) {
	node_t* n = var_find(env, "str");
	if(n->var == NULL || n->var->value == NULL || n->var->type != V_STRING)
		return NULL;

	dump((const char*)n->var->value);
	return NULL;
}


int main(int argc, char** argv) {
		
//	while(true) {
		vm_t vm;
		vm_init(&vm);

		vm_reg_native(&vm, "print(str)", native_print);
		vm_reg_var(&vm, "_HELLO", var_new_str("Hello, world\n"));

		vm_load(&vm, "print(_HELLO);", dump);

		vm_run(&vm);
		vm_close(&vm);
//	}
	return 0;
}
