# Mario
very tiny simple js engine in one single c head file, including bytecode-compiler, VM interpreter. None 3rd libs relied, so can be used on most of embedded systems.

.Demo source.

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

		vm_t vm;
		vm_init(&vm);

		//demo: register a native function(mapped to js).
		vm_reg_native(&vm, "", "print(v)", native_print, NULL);
		//demo: register a global variable.
		vm_reg_var(&vm, "", "_HELLO", var_new_str("Hello, world\n"));

		if(vm_load(&vm, "print(this._HELLO); Debug.dump(this);")) {
			vm_run(&vm);
		}

		vm_close(&vm);
		return 0;
	}
