#ifndef MARIO_HAI_NATIVE_BUILTIN_H
#define MARIO_HAI_NATIVE_BUILTIN_H 

#include "mario_js.h"
#include "basic/native_basic.h"
#include "math/native_math.h"


#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif

static void reg_basic_natives(vm_t* vm) {
	reg_native_basic(vm);
	reg_native_math(vm);
}

#ifdef __cplusplus /* __cplusplus */
}
#endif

#endif
