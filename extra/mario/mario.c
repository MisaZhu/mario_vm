#include "mario_js.h"
#include "hai.h"
#include "hai_native.h"


bool load_js(vm_t* vm, const char* fname) {
	if(_hai_fs.open == NULL)
		return false;

	int fd = _hai_fs.open(fname, _hai_fs.RDONLY);
	if(fd < 0) {
		printf("Can not open file '%s'\n", fname);
		return false;
	}

	int size = _hai_fs.size(fd);

	char* s = (char*)_malloc(size+1);
	_hai_fs.read(fd, s, size);
	_hai_fs.close(fd);
	s[size] = 0;

	bool ret = vm_load(vm, s);
	_free(s);

	return ret;
}

int main(int argc, char** argv) {
	hai_hook();

	_debug_func = _hai_debug.out;

	if(argc != 2) {
		printf("Usage: mario <js-filename>\n");
		return 1;
	}
	

//	while(true) {
	vm_t vm;
	vm_init(&vm);

	reg_natives(&vm);

	if(load_js(&vm, argv[1])) {
#ifndef VERIFY
		vm_run(&vm);
#endif
	}
	
	vm_close(&vm);
//	}
	return 0;
}
