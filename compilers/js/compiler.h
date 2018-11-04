#ifndef MARIO_JS
#define MARIO_JS

#include "mario_vm.h"

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif

bool compiler(bytecode_t *bc, const char* input);

#ifdef __cplusplus /* __cplusplus */
}
#endif


#endif
