#include "mario_js.h"

const char* js = " \
	function hello() { \
		console.log(\"Hello JS world!\n\"); \
	} \
	hello();";

int main(int argc, char** argv) {
	vm_t vm;

	vm_init(&vm); //initialize the vm enviroment.

	if(vm_load(&vm, js)) { // load JS script (and compile to bytecode). 
		vm_run(&vm); //run bytecode.
	}
	
	vm_close(&vm); //release
	return 0;
}
