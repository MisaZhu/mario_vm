#include "mario_js.h"
#include "fs/native_fs.h"
#include "socket/native_socket.h"
#include "system/native_system.h"
#include "math/native_math.h"


#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif

void reg_natives(vm_t* vm) {
	reg_native_fs(vm);
	reg_native_socket(vm);
	reg_native_system(vm);
	reg_native_math(vm);
}

#ifdef __cplusplus /* __cplusplus */
}
#endif
