#include "mario_js.h"
#include "hai.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

var_t* native_debug_out(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "str");
	const char* str = n == NULL ? "" : var_get_str(n->var);
	_hai_debug.out(str);
	return NULL;
}

var_t* native_debug_outln(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "str");
	const char* str = n == NULL ? "" : var_get_str(n->var);
	_hai_debug.outln(str);
	return NULL;
}

#define CLS_DEBUG "Debug"

void reg_native_debug(vm_t* vm) {
	vm_reg_native(vm, CLS_DEBUG, "out(str)", native_debug_out, NULL);
	vm_reg_native(vm, CLS_DEBUG, "outln(str)", native_debug_outln, NULL);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

