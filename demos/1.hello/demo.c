#include "mario_js.h"
#include <stdio.h>

const char* js = " \
	function hello() { \
		console.log(\"Hello JS world!\n\"); \
	} \
	hello();";

int main(int argc, char** argv) {
	vm_t vm;
	vm_init(&vm);

	if(vm_load(&vm, js)) {
		vm_run(&vm);
	}
	
	vm_close(&vm);
	return 0;
}
