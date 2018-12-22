#include "mario_vm.h"
#include "native_builtin.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdio.h>

#define ERR_MAX 1023
char _err_info[ERR_MAX+1];

bool load_js(vm_t* vm, const char* fname, bool verify) {
	int fd = open(fname, O_RDONLY);
	if(fd < 0) {
		snprintf(_err_info, ERR_MAX, "Can not open file '%s'\n", fname);
		_err(_err_info);
		return false;
	}

	struct stat st;
	fstat(fd, &st);

	char* s = (char*)_malloc(st.st_size+1);
	int sz = read(fd, s, st.st_size);
	close(fd);
	
	if(sz != st.st_size) {
		_free(s);
		return false;
	}

	s[sz] = 0;

	bool ret;
	if(verify)
		ret = vm_load(vm, s);
	else
		ret = vm_load_run(vm, s);
	_free(s);

	return ret;
}

/**
load extra native libs.
*/
typedef void (*reg_natives_t)(vm_t* vm);

#define MAX_EXTRA 8
void* libs[MAX_EXTRA];
int libs_num = 0;

bool load_extra(const char* n) {
	if(libs_num >= MAX_EXTRA) {
		_err("Too many extended module loaded!\n");
		return false;
	}

	void* h = dlopen(n, RTLD_LAZY|RTLD_GLOBAL);
	if(h == NULL) {
		const char* e = dlerror();
		snprintf(_err_info, ERR_MAX, "Extended module load error(%s)!%s\n", n, e != NULL? e:"");
		_err(_err_info);
		return false;
	}

	libs[libs_num++] = h;
	return true;
}

void reg_natives(vm_t* vm) {
	reg_basic_natives(vm);
	int i;
	for(i=0; i<libs_num; i++) {
		void* h = libs[i];
		reg_natives_t loader = (reg_natives_t)dlsym(h, "reg_natives");
		loader(vm);
	}
}

void unload_extra(vm_t* vm) {
	int i;
	for(i=0; i<libs_num; i++) {
		dlclose(libs[i]);
	}
}

#define DEF_LIBS "/usr/local/mario"

bool load_natives() {
	const char* path = getenv("MARIO_PATH");
	if(path == NULL)
		path = DEF_LIBS;
	
	char fpath[1024];
	snprintf(fpath, 1023, "%s/natives", path);

	DIR* dir = opendir(fpath);
	if(dir == NULL) {
		snprintf(_err_info, ERR_MAX, "Warning: MARIO_LIBS does't exist('%s'), skip loading extra natives!\n", path);
		_debug(_err_info);
		return true;
	}

	bool ret = true;
	char fname[1024];
	while(true) {
		struct dirent* dp = readdir(dir);
		if(dp == NULL)
			break;
		
		if(strstr(dp->d_name, ".so") == NULL)
			continue;
	
		snprintf(fname, 1023, "%s/%s", fpath, dp->d_name);
		
		snprintf(_err_info, ERR_MAX, "Loading native lib %s ......", fname);
		if(!load_extra(fname)) {
			_err(_err_info);
			_err(" failed!\n");
			ret = false;
			break;
		}

		_debug(_err_info);
		_debug(" ok.\n");
	}
	
	closedir(dir);
	return ret;
}

bool load_script_libs(vm_t* vm, bool verify) {
	const char* path = getenv("MARIO_PATH");
	if(path == NULL)
		path = DEF_LIBS;

	str_t* fpath = str_new(path);
	str_append(fpath, "/libs/");
	str_append(fpath, _mario_lang);

	DIR* dir = opendir(fpath->cstr);
	if(dir == NULL) {
		snprintf(_err_info, ERR_MAX, "Warning: MARIO_LIBS does't exist('%s'), skip loading extra js libs!\n", path);
		_debug(_err_info);
		str_free(fpath);
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
	
		str_cpy(fname, fpath->cstr);
		str_add(fname, '/');
		str_append(fname, dp->d_name);
		
		snprintf(_err_info, ERR_MAX, "Loading js lib %s ......", fname->cstr);
		if(!load_js(vm, fname->cstr, verify)) {
			_err(_err_info);
			_err(" failed!\n");
			ret = false;
			break;
		}
		_debug(_err_info);
		_debug(" ok.\n");
	}
	
	str_free(fpath);
	str_free(fname);
	closedir(dir);
	return ret;
}

void init_args(vm_t* vm, int argc, char** argv) {
	var_t* args = var_new_array(vm);
	int i;
	for(i=0; i<argc; i++) {
		var_array_add(args, var_new_str(vm, argv[i]));
	}

	var_add(vm->root, "_args", args);
}

void run_shell(vm_t* vm);

int main(int argc, char** argv) {
	/*if(argc < 2) {
		_err("Usage: mario (-v) <js-filename>\n");
	}
	*/

	bool verify = false;
	const char* fname = "";

	if(argc > 1) {
		if(strcmp(argv[1], "-v") == 0) {
			if(argc != 3)
				return 1;
			verify = true;
			fname = argv[2];
		}
		else if(strcmp(argv[1], "-d") == 0) {
#ifndef MARIO_DEBUG
			_err("Error: Can't run debug mode, try rebuild with 'export MARIO_DEBUG=yes'.\n");
			return 1;
#endif
			_debug_mode = true;
			if(argc == 3)
				fname = argv[2];
		}
		else {
			fname = argv[1];
		}
	}
	
	bool loaded = true;
	//load extra native so files.
	if(!load_natives()) {
		loaded = false;
	}

	_mem_init();
	vm_t* vm = vm_new(compile);
	vm->gc_buffer_size = 1024;

	init_args(vm, argc, argv);

	//load extra js files.
	if(loaded) {
		if(!load_script_libs(vm, verify))
			loaded = false;
	}

	if(loaded) {
		vm_init(vm, reg_natives, NULL);

		if(fname[0] != 0) {
			_debug("-------- run script --------\n");
			if(load_js(vm, fname, verify)) {
				if(verify)
					vm_dump(vm);
			}
		}
		else {
			run_shell(vm);
		}
	}
	
	vm_close(vm);
	_mem_close();
	return 0;
}
