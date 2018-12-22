#include "mario_vm.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

var_t* native_demo_test(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	const char* str = get_str(env, "str");
	_debug(str);
	return NULL;
}

void reg_natives(vm_t* vm) {
	vm_reg_native(vm, "Demo", "test(str)", native_demo_test, NULL);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

