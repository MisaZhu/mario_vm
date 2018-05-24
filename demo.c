#include "mario.h"
#include <stdio.h>

void dump(const char* s) {
	printf("%s", s);
}

int main(int argc, char** argv) {
	const char* s = "var abc = 1;";
	while(true) {
	vm_t vm;
	vm_init(&vm);
	vm_load(&vm, s, dump);
	vm_run(&vm);
	vm_close(&vm);
	}
	return 0;
}
