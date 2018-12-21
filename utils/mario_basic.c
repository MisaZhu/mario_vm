#include "mario_basic.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void _free_none(void* p) { (void)p; }

void default_out(const char* s) {
	printf("%s", s);
}

void (*_out_func)(const char*) = default_out;
bool _debug_mode = false;

void _debug(const char* s) {
	if(_debug_mode)
		_out_func(s);
}

void _err(const char* s) {
	_out_func(s);
}

//#ifdef MARIO_DEBUG
/*mem_block_t* _mem_head = NULL;
pthread_mutex_t _mem_lock;

inline void* _raw_malloc(uint32_t size, const char* file, uint32_t line) {
	if(size == 0)
		return NULL;

	pthread_mutex_lock(&_mem_lock);
	mem_block_t* block = (mem_block_t*)malloc(sizeof(mem_block_t));
	block->p = malloc(size);
	block->size = size;
	block->file = file;
	block->line = line;
	block->prev = NULL;

	if(_mem_head != NULL)
		_mem_head->prev = block;
	block->next = _mem_head;
	_mem_head = block;
	pthread_mutex_unlock(&_mem_lock);
	return block->p;
}

inline void _free(void* p) {
	pthread_mutex_lock(&_mem_lock);
	mem_block_t* block = _mem_head;	
	while(block != NULL) {
		if(block->p == p) // found.
			break;
		block = block->next;
	}

	if(block == NULL) {
		pthread_mutex_unlock(&_mem_lock);
		return;
	}
	
	if(block->next != NULL)
		block->next->prev = block->prev;
	if(block->prev != NULL)
		block->prev->next = block->next;
	
	if(block == _mem_head)
		_mem_head = block->next;

	free(block->p);
	free(block);
	pthread_mutex_unlock(&_mem_lock);
}

void _mem_init() { 
	_mem_head = NULL;	
	pthread_mutex_init(&_mem_lock, NULL);
}

void _mem_close() { 
	pthread_mutex_lock(&_mem_lock);
	mem_block_t* block = _mem_head;	
	if(block == NULL) { // mem clean
		_debug("memory is cleaned up.\n");
	}
	else {
		_debug("memory is leaking!!!\n");
		while(block != NULL) {
			char tmp[STATIC_STR_MAX];
			_debug(" ");
			_debug(block->file);
			_debug(", ");
			_debug(str_from_int(block->line, tmp, 10));
			_debug(", size=");
			_debug(str_from_int(block->size, tmp, 10));
			_debug("\n");
			block = block->next;
		}
	}
	pthread_mutex_unlock(&_mem_lock);

	pthread_mutex_destroy(&_mem_lock);
}

void *_raw_realloc(void* p, uint32_t old_size, uint32_t new_size, const char* file, uint32_t line) {
	void *np = _raw_malloc(new_size, file, line);
	if(p != NULL && old_size > 0) {
		memcpy(np, p, old_size);
		_free(p);
	}
	return np;
}
*/
//#else

void _mem_init() { }
void _mem_close() { }

void *_raw_realloc(void* p, uint32_t old_size, uint32_t new_size, const char* file, uint32_t line) {
	void *np = _malloc(new_size);
	if(p != NULL && old_size > 0) {
		memcpy(np, p, old_size);
		_free(p);
	}
	return np;
}

//#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

