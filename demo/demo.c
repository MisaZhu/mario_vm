#include "mario_js.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

void* interrupterThread(void* arg) {
	vm_t* vm = (vm_t*)arg;
	
	pthread_detach(pthread_self());

	while(true) {
		node_t* func = find_member(vm->root, "onInterrupt");
		if(func != NULL)
			interrupt(vm, vm->root, func, NULL);

		usleep(10000);
	}
	return NULL;
}

void debug(const char* s) {
	printf("%s", s);
}

const char* js = " \
	var a = 0; \
	function onInterrupt() { \
		dump(\"interrupter: \" + a); \
		a++; \
	} \
	while(true) { \
		yield(); \
	}";

int main(int argc, char** argv) {
	_debug_func = debug;

	vm_t vm;
	vm_init(&vm);

	pthread_t pth;
	pthread_create(&pth, NULL, interrupterThread, &vm);

	if(vm_load(&vm, js)) {
		vm_run(&vm);
	}
	
	vm_close(&vm);
	return 0;
}
