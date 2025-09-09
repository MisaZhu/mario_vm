#include "mario.h"
#include "platform.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void* (*_platform_malloc)(uint32_t size) = NULL;
void  (*_platform_free)(void* p) = NULL;

static void out(const char* str) {
    write(1, str, strlen(str));
}

void platform_init(void) {
	_platform_malloc = malloc;
	_platform_free = free;
    _out_func = out;
}

#ifdef __cplusplus
}
#endif
