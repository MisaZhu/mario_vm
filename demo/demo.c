#include "mario_js.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

void* interrupterThread(void* arg) {
	vm_t* vm = (vm_t*)arg;
	
	pthread_detach(pthread_self());
	
	int32_t count1 = 0;
	int32_t count2 = 0;

	while(true) {
		//find function name onInterrupt with 1 argument.
		var_t* args = var_new();
		var_add(args, "", var_new_int(count1)); //the first argment
		var_add(args, "", var_new_int(count2)); //the second argment
		interrupt(vm, vm->root, "onInterrupt", args);

		usleep(100);
		count1++;
		count2 += 2;
	}
	return NULL;
}

void debug(const char* s) {
	printf("%s", s);
}

const char* js = " \
	function onInterrupt(count1, count2) { \
		dump(\"interrupter: \" + count1 + \": \" + count2); \
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
