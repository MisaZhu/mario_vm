#include "mario_js.h"

const char* js = " \
	function jsFunc() { \
		return \"Hello JS world!\n\"; \
	} \
	";

int main(int argc, char** argv) {
	vm_t vm;
	vm_init(&vm); //initialize the vm enviroment.

	if(vm_load(&vm, js)) { // load JS script (and compile to bytecode). 
		vm_run(&vm);

		var_t* ret = call_js_func_by_name(&vm, vm.root, "jsFunc", NULL);
		if(ret != NULL) {
			const char* s = var_get_str(ret);
			_out_func(s);
			var_unref(ret, true);
		}
	}
	
	vm_close(&vm); //release
	return 0;
}
