#include "mario.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

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

bool load_js(vm_t* vm, const char* fname) {
	int fd = open(fname, O_RDONLY);
	if(fd < 0) {
		printf("Can not open file '%s'\n", fname);
		return false;
	}

	struct stat st;
	fstat(fd, &st);

	char* s = (char*)_malloc(st.st_size+1);
	read(fd, s, st.st_size);
	close(fd);
	s[st.st_size] = 0;

	vm_load(vm, s, dump);
	_free(s);

	return true;
}

int main(int argc, char** argv) {
	if(argc != 2) {
		printf("Usage: mario <js-filename>\n");
		return 1;
	}
	//while(true) {
	vm_t vm;
	vm_init(&vm);

	vm_reg_native(&vm, "print(str)", native_print);

	load_js(&vm, argv[1]);

	vm_run(&vm);
	vm_close(&vm);
	//}
	return 0;
}
