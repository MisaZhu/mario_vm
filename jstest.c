#include "mario_js.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void debug(const char* s) {
	printf("%s", s);
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

	bool ret = vm_load(vm, s);
	_free(s);

	return ret;
}

int main(int argc, char** argv) {
	_debug_func = debug;

	if(argc < 2) {
		printf("Usage: mario <js-filename>\n");
		return 1;
	}

//	while(true) {
	vm_t vm;
	vm_init(&vm);

	if(load_js(&vm, argv[1])) {
		vm_run(&vm);
	}
	
	vm_close(&vm);
//	}
	return 0;
}
