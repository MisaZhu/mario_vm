#ifdef __cplusplus
extern "C" {
#endif

#include "native_console.h"

static str_t* args_to_str(var_t* args) {
	str_t* ret = str_new("");
	str_t* str = str_new("");
	uint32_t sz = var_array_size(args);
	uint32_t i;
	for(i=0; i<sz; ++i) {
		node_t* n = var_array_get(args, i);
		if(n != NULL) {
			var_to_str(n->var, str);
			if(i > 0)
				str_add(ret, ' ');
			str_append(ret, str->cstr);
		}
	}
	str_free(str);
	return ret;
}

var_t* native_print(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* v = get_func_args(env); 
	str_t* ret = args_to_str(v);
	_out_func(ret->cstr);
	str_free(ret);
	return NULL;
}

var_t* native_println(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* v = get_func_args(env); 
	str_t* ret = args_to_str(v);
	str_add(ret, '\n');
	_out_func(ret->cstr);
	str_free(ret);
	return NULL;
}

#define CLS_CONSOLE "console"

void reg_native_console(vm_t* vm) {
	var_t* cls = vm_new_class(vm, CLS_CONSOLE);
	vm_reg_static(vm, cls, "write(v)", native_print, NULL); 
	vm_reg_static(vm, cls, "log(v)", native_println, NULL); 
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
