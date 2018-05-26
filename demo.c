#include "mario.h"
#include <stdio.h>
#include <unistd.h>

void dump(const char* s) {
	printf("%s", s);
}

var_t* native_print(vm_t* vm, var_t* env, void* data) {
	node_t* n = var_find(env, "str");
	if(n->var == NULL || n->var->value == NULL || n->var->type != V_STRING)
		return NULL;

	dump((const char*)n->var->value);
	return NULL;
}

var_t* native_delay(vm_t* vm, var_t* env, void* data) {
  node_t* n = var_find(env, "msec");
  int msec = *(int*)n->var->value;

  usleep(msec);
  return NULL;
}

int main(int argc, char** argv) {
	const char* s = "var i=0; while(i<10) { delay(10000);	print('.\n'); i++; }";
//	while(true) {
		vm_t vm;
		vm_init(&vm);

		vm_reg_native(&vm, "print(str)", native_print);
		vm_reg_native(&vm, "delay(msec)", native_delay);

		vm_load(&vm, s, dump);
		vm_run(&vm);
		vm_close(&vm);
//	}
	return 0;
}
