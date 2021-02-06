#include "mario.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

var_t* native_demo_test(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	const char* str = get_str(env, "str");
	mario_debug(str);
	return NULL;
}

void reg_natives(vm_t* vm) {
	var_t* cls = vm_new_class(vm, "Demo");
	vm_reg_native(vm, cls, "test(str)", native_demo_test, NULL);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

