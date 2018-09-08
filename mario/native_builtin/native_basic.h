#ifndef MARIO_HAI_NATIVE_BASIC_H
#define MARIO_HAI_NATIVE_BASIC_H 

#include "mario_js.h"
#include "basic/native_basic.h"


#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif

static void reg_basic_natives(vm_t* vm) {
	reg_native_basic(vm);
}

#ifdef __cplusplus /* __cplusplus */
}
#endif

#endif
