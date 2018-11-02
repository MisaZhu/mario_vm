#include "mario_js.h"

const char* js = " \
	function hello() { \
		console.log(\"Hello JS world!\n\"); \
	} \
	hello();";

int main(int argc, char** argv) {
	vm_t* vm = vm_new();

	vm_init(vm, NULL, NULL); //initialize the vm enviroment.

	vm_load_run(vm, js); // load JS script (and compile to bytecode) and run
	
	vm_close(vm); //release
	return 0;
}
