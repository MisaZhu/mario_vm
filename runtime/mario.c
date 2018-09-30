#include "mario_js.h"
#include "native_basic.h"

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>

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

/**
load extra native libs.
*/
typedef void (*reg_natives_t)(vm_t* vm);

#define MAX_EXTRA 8
void* libs[MAX_EXTRA];
int libsNum = 0;

bool loadExtra(const char* n, vm_t* vm) {
	if(libsNum >= MAX_EXTRA) {
		printf("Too many extended module loaded!\n");
		return false;
	}

	void* h = dlopen(n, RTLD_LAZY);
	if(h == NULL) {
		const char* e = dlerror();
		printf("Extended module load error(%s)!%s\n", n, e != NULL? e:"");
		return false;
	}

	reg_natives_t loader = (reg_natives_t)dlsym(h, "reg_natives");
	if(loader == NULL) {
		const char* e = dlerror();
		printf("Extended module load-function dosen't exist(%s)!%s\n", n, e != NULL? e:"");
		dlclose(h);
		return false;
	}

	loader(vm);
	libs[libsNum++] = h;
	return true;
}

void unloadExtra() {
	int i;
	for(i=0; i<libsNum; i++) {
		dlclose(libs[i]);
	}
}

int main(int argc, char** argv) {

	if(argc < 2) {
		printf("Usage: mario <js-filename> (.so native_files)\n");
		return 1;
	}

	bool verify = false;
	const char* fname;
	int soindex;

	if(strcmp(argv[1], "-v") == 0) {
		if(argc != 3)
			return 1;
		verify = true;
		fname = argv[2];
		soindex = 3;
	}
	else {
		fname = argv[1];
		soindex = 2;
	}
	
	vm_t vm;
	vm_init(&vm);

	reg_basic_natives(&vm);
	
	//load extra native so files.
	bool loaded = true;
	for(; soindex < argc; soindex++) {
		if(!loadExtra(argv[soindex], &vm)) {
			loaded = false;
			break;
		}
	}

	if(loaded) {
		if(load_js(&vm, fname) && !verify) {
			vm_run(&vm);
		}
	}
	
	vm_close(&vm);
	unloadExtra();
	return 0;
}
