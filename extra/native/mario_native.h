#ifndef MARIO_NATIVE_H
#define MARIO_NATIVE_H

#include "mario_js.h"
#include "basic/native_basic.h"
#include "debug/native_debug.h"
#include "fs/native_fs.h"

static void reg_natives(vm_t* vm) {
	reg_native_basic(vm);
	reg_native_debug(vm);
	reg_native_fs(vm);
}

#endif
