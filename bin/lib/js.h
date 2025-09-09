#ifndef MARIO_JS_H
#define MARIO_JS_H

#include "mario.h"

#ifdef __cplusplus
extern "C" {
#endif

bool load_js(vm_t* vm, const char* fname);

#ifdef __cplusplus
}
#endif

#endif