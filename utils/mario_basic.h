#ifndef MARIO_BASIC
#define MARIO_BASIC

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#else

#ifndef bool
typedef enum bool_enum {false, true} bool;
#endif
#endif

/** memory functions.-----------------------------*/
//#ifndef MARIO_DEBUG
	#define _malloc malloc
	#define _free free
//#else
	/*#include <pthread.h>
	typedef struct mem_block {
		void* p;
		uint32_t size;
		const char* file;
		uint32_t line;
		struct mem_block *prev;
		struct mem_block *next;
	} mem_block_t;

	extern mem_block_t* _mem_head;
	extern pthread_mutex_t _mem_lock;

	extern void* _raw_malloc(uint32_t size, const char* file, uint32_t line);
	#define _malloc(size) _raw_malloc((size), __FILE__, __LINE__)
	extern void _free(void *p);*/
//#endif

extern void _mem_init();
extern void _mem_close();
extern void* _raw_realloc(void* p, uint32_t old_size, uint32_t new_size, const char* file, uint32_t line);
#define _realloc(p, old_size, new_size) _raw_realloc(p, old_size, new_size, __FILE__, __LINE__)

#define STATIC_STR_MAX 32

typedef void (*free_func_t)(void* p);

extern void _free_none(void*p);

extern void (*_out_func)(const char*);
extern bool _debug_mode;
void _debug(const char* s);
void _err(const char* s);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
