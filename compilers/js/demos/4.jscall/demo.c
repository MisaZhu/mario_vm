#include "mario_vm.h"

const char* js = " \
	function jsFunc() { \
		return \"Hello JS world!\n\"; \
	} \
	";

int main(int argc, char** argv) {
	vm_t* vm = vm_new();
	vm_init(vm, compile, NULL, NULL); //initialize the vm enviroment.

	vm_load_run(vm, js); 

	var_t* ret = call_m_func_by_name(vm, vm->root, "jsFunc", NULL);
	if(ret != NULL) {
		const char* s = var_get_str(ret);
		_out_func(s);
		var_unref(vm, ret, true);
	}
	
	vm_close(vm); //release
	return 0;
}
