#include "mario_js.h"
#include "hai_fs.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*=====fs native functions=========*/
var_t* native_fs_close(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	if(_hai_fs.close == NULL) {
		return NULL;
	}

	node_t* n = var_find(env, "fd");
	int fd = n == NULL ? 0 : var_get_int(n->var);

	_hai_fs.close(fd);
	return NULL;
}

var_t* native_fs_open(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	if(_hai_fs.open == NULL) {
		return NULL;
	}

	node_t* n = var_find(env, "fname");
	const char* fname = n == NULL ? "" : var_get_str(n->var);
	n = var_find(env, "flags");
	int flags = n == NULL ? 0 : var_get_int(n->var);

	int res = _hai_fs.open(fname, flags);
	return var_new_int(res);
}

var_t* native_fs_write(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	if(_hai_fs.write == NULL) {
		return NULL;
	}

	node_t* n = var_find(env, "fd");
	int fd = n == NULL ? 0 : var_get_int(n->var);

	n = var_find(env, "bytes");
	if(n == NULL || n->var == NULL || n->var->size == 0)
		return NULL;
	var_t* bytes = n->var;

	int bytesSize = bytes->size;
	if(bytes->type == V_STRING)
		bytesSize--;

	n = var_find(env, "size");
	int size = n == NULL ? 0 : var_get_int(n->var);
	if(size > bytesSize)
		size = bytesSize;

	int res = _hai_fs.write(fd, (const char*)bytes->value, size);
	return var_new_int(res);
}

var_t* native_fs_read(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	if(_hai_fs.read == NULL) {
		return NULL;
	}

	node_t* n = var_find(env, "fd");
	int fd = n == NULL ? 0 : var_get_int(n->var);

	n = var_find(env, "bytes");
	if(n == NULL || n->var == NULL || n->var->size == 0)
		return NULL;
	var_t* bytes = n->var;

	int bytesSize = bytes->size;

	n = var_find(env, "size");
	int size = n == NULL ? 0 : var_get_int(n->var);
	if(size > bytesSize)
		size = bytesSize;

	int res = _hai_fs.read(fd, (char*)bytes->value, size);
	return var_new_int(res);
}

#define CLS_FS "FS"

void reg_native_fs(vm_t* vm) {
	vm_reg_var(vm, CLS_FS, "RDONLY", var_new_int(_hai_fs.RDONLY));
	vm_reg_var(vm, CLS_FS, "WRONLY", var_new_int(_hai_fs.WRONLY));
	vm_reg_var(vm, CLS_FS, "RDWR", var_new_int(_hai_fs.RDWR));

	vm_reg_native(vm, CLS_FS, "close(fd)", native_fs_close, NULL);
	vm_reg_native(vm, CLS_FS, "open(fname, flags)", native_fs_open, NULL);
	vm_reg_native(vm, CLS_FS, "write(fd, bytes, size)", native_fs_write, NULL);
	vm_reg_native(vm, CLS_FS, "read(fd, bytes, size)", native_fs_read, NULL);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

