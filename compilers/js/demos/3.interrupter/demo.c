#include "mario_vm.h"
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
		var_t* args = var_new(); //interrupter function arguments.
		var_add(vm, args, "", var_new_int(count1)); //the first argment
		var_add(vm, args, "", var_new_int(count2)); //the second argment

		//call interrupter 'onInterrupt' with 2 argument.
		interrupt_by_name(vm, vm->root, "onInterrupt", args);

		usleep(100);
		count1++;
		count2 += 2;
	}
	return NULL;
}

const char* js = " \
	function onInterrupt(count1, count2) { \
		console.ln(\"interrupter: \" + count1 + \": \" + count2); \
	} \
	while(true) { \
		yield(); \
	}";

int main(int argc, char** argv) {
	vm_t* vm = vm_new(compile);
	vm_init(vm, NULL, NULL);

	pthread_t pth;
	pthread_create(&pth, NULL, interrupterThread, vm);

	vm_load_run(vm, js);
	
	vm_close(vm);
	return 0;
}
