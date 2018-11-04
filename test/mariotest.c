#include "mario_vm.h"

#include <unistd.h>
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
	read(fd, s, st.st_size);
	close(fd);
	s[st.st_size] = 0;

	bool ret;
	if(verify)
		ret = vm_load(vm, s);
	else
		ret = vm_load_run(vm, s);
	_free(s);

	return ret;
}

void run_shell(vm_t* vm) {
	str_t* cmd = str_new("");
	while(!vm->terminated) {
		str_reset(cmd);

		write(0, ":> ", 3);
		while(true) {
			char c[1];
			if(read(0, c, 1) <= 0) {
				vm->terminated = true;
				break;
			}
			str_add(cmd, c[0]);
			if(c[0] == '\n')
				break;
		}

		if(cmd->len > 0 && cmd->cstr[0] != '\n') {
			if(strncmp(cmd->cstr, "exit\n", 5) == 0) {
				vm->terminated = true;
				break;
			}
			else {
				vm_load_run(vm, cmd->cstr);
			}
		}
	}
	str_free(cmd);
}

int main(int argc, char** argv) {
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
			if(argc != 3)
				return 1;
			_debug_mode = true;
			fname = argv[2];
		}
		else {
			fname = argv[1];
		}
	}
	
	vm_t* vm = vm_new();
	vm_init(vm, compile, NULL, NULL);

	if(fname[0] != 0) {
		if(load_js(vm, fname, verify)) {
			if(verify)
				vm_dump(vm);
		}
	}
	else {
		run_shell(vm);
	}

	vm_close(vm);
	return 0;
}
