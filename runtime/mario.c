#include "mario_js.h"
#include "builtin/native_builtin.h"

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdio.h>

#define ERR_MAX 1023
char _errInfo[ERR_MAX+1];

bool load_js(vm_t* vm, const char* fname) {
	int fd = open(fname, O_RDONLY);
	if(fd < 0) {
		snprintf(_errInfo, ERR_MAX, "Can not open file '%s'\n", fname);
		_err(_errInfo);
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

bool loadExtra(vm_t* vm, const char* n) {
	if(libsNum >= MAX_EXTRA) {
		_err("Too many extended module loaded!\n");
		return false;
	}

	void* h = dlopen(n, RTLD_LAZY);
	if(h == NULL) {
		const char* e = dlerror();
		snprintf(_errInfo, ERR_MAX, "Extended module load error(%s)!%s\n", n, e != NULL? e:"");
		_err(_errInfo);
		return false;
	}

	reg_natives_t loader = (reg_natives_t)dlsym(h, "reg_natives");
	if(loader == NULL) {
		const char* e = dlerror();
		snprintf(_errInfo, ERR_MAX, "Extended module load-function dosen't exist(%s)!%s\n", n, e != NULL? e:"");
		_err(_errInfo);
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

#define DEF_LIBS "/usr/local/mario/libs"

bool load_natives(vm_t* vm) {
	const char* path = getenv("MARIO_LIBS");
	if(path == NULL)
		path = DEF_LIBS;

	DIR* dir = opendir(path);
	if(dir == NULL) {
		snprintf(_errInfo, ERR_MAX, "Warning: MARIO_LIBS does't exist('%s'), skip loading extra natives!\n", path);
		_err(_errInfo);
		return true;
	}

	bool ret = true;
	str_t* fname = str_new("");
	while(true) {
		struct dirent* dp = readdir(dir);
		if(dp == NULL)
			break;
		
		if(strstr(dp->d_name, ".so") == NULL)
			continue;
	
		str_cpy(fname, path);
		str_add(fname, '/');
		str_append(fname, dp->d_name);
		
		snprintf(_errInfo, ERR_MAX, "Loading native lib %s ......", fname->cstr);
		if(!loadExtra(vm, fname->cstr)) {
			_err(_errInfo);
			_err(" failed!\n");
			ret = false;
			break;
		}

		_debug(_errInfo);
		_debug(" ok.\n");
	}
	
	str_free(fname);
	closedir(dir);
	return ret;
}

bool load_js_libs(vm_t* vm) {
	const char* path = getenv("MARIO_LIBS");
	if(path == NULL)
		path = DEF_LIBS;

	DIR* dir = opendir(path);
	if(dir == NULL) {
		snprintf(_errInfo, ERR_MAX, "Warning: MARIO_LIBS does't exist('%s'), skip loading extra natives!\n", path);
		_err(_errInfo);
		return true;
	}

	bool ret = true;
	str_t* fname = str_new("");
	while(true) {
		struct dirent* dp = readdir(dir);
		if(dp == NULL)
			break;
		
		if(strstr(dp->d_name, ".js") == NULL)
			continue;
	
		str_cpy(fname, path);
		str_add(fname, '/');
		str_append(fname, dp->d_name);
		
		snprintf(_errInfo, ERR_MAX, "Loading js lib %s ......", fname->cstr);
		if(!load_js(vm, fname->cstr)) {
			_err(_errInfo);
			_err(" failed!\n");
			ret = false;
			break;
		}
		_debug(_errInfo);
		_debug(" ok.\n");
	}
	
	str_free(fname);
	closedir(dir);
	return ret;
}

void init_args(vm_t* vm, int argc, char** argv) {
	var_t* args = var_new_array();
	int i;
	for(i=0; i<argc; i++) {
		var_add(args, "", var_new_str(argv[i]));
	}

	var_add(vm->root, "_args", args);
}

int main(int argc, char** argv) {

	if(argc < 2) {
		_err("Usage: mario (-v) <js-filename>\n");
		return 1;
	}

	bool verify = false;
	const char* fname;

	if(strcmp(argv[1], "-v") == 0) {
		if(argc != 3)
			return 1;
		verify = true;
		fname = argv[2];
	}
	else if(strcmp(argv[1], "-d") == 0) {
		if(argc != 3)
			return 1;
		_debugMode = true;
		fname = argv[2];
	}
	else {
		fname = argv[1];
	}
	
	vm_t vm;
	vm_init(&vm);

	init_args(&vm, argc, argv);

	reg_basic_natives(&vm);
	
	//load extra native so files.
	bool loaded = true;

	if(!load_natives(&vm)) {
		loaded = false;
	}

	if(loaded) {
		if(!load_js_libs(&vm))
			loaded = false;
	}

	if(loaded) {
		_debug("-------- run js --------\n");
		if(load_js(&vm, fname)) {
			if(verify)
				vm_dump(&vm);
			else
				vm_run(&vm);
		}
	}
	
	vm_close(&vm);
	unloadExtra();
	return 0;
}
