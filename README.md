# Mario
very tiny simple js engine in one single c  and one single head file "mario_js.c mario_js.h", including bytecode-compiler, VM interpreter. None 3rd libs relied, so can be used on most of embedded systems.

.Demo source.

	#include "mario_js.h"

	var_t* native_out(vm_t* vm, var_t* env, void* data) {
		const char* s = get_str(env, "v");
		_out_func(s);
		return NULL;
	}

	int main(int argc, char** argv) {
		vm_t vm;
		vm_init(&vm);

		//demo: register a native function(mapped to js).
		vm_reg_native(&vm, "", "out(v)", native_out, NULL);

		//demo: register a const variable in class.
		vm_reg_var(&vm, "ClassHello", "HELLO", var_new_str("Hello, native world!\n"), true);

		if(vm_load(&vm, "out(ClassHello.HELLO);")) {
			vm_run(&vm);
		}

		vm_close(&vm);
		return 0;
	}

