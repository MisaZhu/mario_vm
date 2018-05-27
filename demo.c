#include "mario.h"
#include <stdio.h>

void debug(const char* s) {
	printf("%s", s);
}

var_t* native_print(vm_t* vm, var_t* env, void* data) {
	node_t* n = var_find(env, "v");
	const char* s = n == NULL ? "" : var_get_str(n->var);
	debug(s);
	return NULL;
}

int main(int argc, char** argv) {
	_debug_func = debug; //for dump variable
		
//	while(true) {
		vm_t vm;
		vm_init(&vm);

		//demo: register a native function(mapped to js).
		vm_reg_native(&vm, "print(v)", native_print, NULL);
		//demo: register a global variable.
		vm_reg_var(&vm, "_HELLO", var_new_str("Hello, world\n"));

		vm_load(&vm, "print(_HELLO);");
		vm_run(&vm);

		vm_close(&vm);
//	}
	return 0;
}
