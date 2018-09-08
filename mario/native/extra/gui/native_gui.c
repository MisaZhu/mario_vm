#include "mario_js.h"
#include "sdl/native_sdl.h"


#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif

void reg_natives(vm_t* vm) {
	reg_native_sdl(vm);
}

#ifdef __cplusplus /* __cplusplus */
}
#endif
