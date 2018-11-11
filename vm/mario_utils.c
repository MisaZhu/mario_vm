#include "mario_utils.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void _free_none(void* p, void* extra) { (void)p; (void)extra; }

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
			_debug(str_from_int(block->line, tmp));
			_debug(", size=");
			_debug(str_from_int(block->size, tmp));
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

/** array functions.-----------------------------*/

#define ARRAY_BUF 16

inline void array_init(m_array_t* array) { 
	array->items = NULL; 
	array->size = 0; 
	array->max = 0; 
}

inline void array_add(m_array_t* array, void* item) {
	uint32_t new_size = array->size + 1; 
	if(array->max <= new_size) { 
		new_size = array->size + ARRAY_BUF;
		array->items = (void**)_realloc(array->items, array->max*sizeof(void*), new_size*sizeof(void*)); 
		array->max = new_size; 
	} 
	array->items[array->size] = item; 
	array->size++; 
	array->items[array->size] = NULL; 
}

void* array_add_buf(m_array_t* array, void* s, uint32_t sz) {
	void* item = _malloc(sz);
	if(s != NULL)
		memcpy(item, s, sz);
	array_add(array, item);
	return item;
}

inline void* array_get(m_array_t* array, uint32_t index) {
	if(array->items == NULL || index >= array->size)
		return NULL;
	return array->items[index];
}

inline void* array_set(m_array_t* array, uint32_t index, void* p) {
	if(array->items == NULL || index >= array->size)
		return NULL;
	array->items[index] = p;
	return p;
}

inline void* array_head(m_array_t* array) {
	if(array->items == NULL || array->size == 0)
		return NULL;
	return array->items[0];
}

inline void* array_remove(m_array_t* array, uint32_t index) { //remove out but not free
	if(index >= array->size)
		return NULL;

	void *p = array->items[index];
	uint32_t i;
	for(i=index; (i+1)<array->size; i++) {
		array->items[i] = array->items[i+1];	
	}

	array->size--;
	array->items[array->size] = NULL;
	return p;
}

inline void array_del(m_array_t* array, uint32_t index, free_func_t fr, void* extra) { // remove out and free.
	void* p = array_remove(array, index);
	if(p != NULL) {
		if(fr != NULL)
			fr(p, extra);
		else
			_free(p);
	}
}

inline void array_remove_all(m_array_t* array) { //remove all items bot not free them.
	if(array->items != NULL) {
		_free(array->items);
		array->items = NULL;
	}
	array->max = array->size = 0;
}

inline void array_clean(m_array_t* array, free_func_t fr, void* extra) { //remove all items and free them.
	if(array->items != NULL) {
		uint32_t i;
		for(i=0; i<array->size; i++) {
			void* p = array->items[i];
			if(p != NULL) {
				if(fr != NULL)
					fr(p, extra);
				else
					_free(p);
			}
		}
		_free(array->items);
		array->items = NULL;
	}
	array->max = array->size = 0;
}

/** str functions.-----------------------------*/

#define STR_BUF 16

void str_reset(str_t* str) {
	if(str->cstr == NULL) {
		str->cstr = (char*)_malloc(STR_BUF);
		str->max = STR_BUF;
	}

	str->cstr[0] = 0;
	str->len = 0;	
}

char* str_ncpy(str_t* str, const char* src, uint32_t l) {
	if(src == NULL || src[0] == 0 || l == 0) {
		str_reset(str);
		return str->cstr;
	}

	uint32_t len = strlen(src);
	if(len > l)
		len = l;

	uint32_t new_size = len;
	if(str->max <= new_size) {
		new_size = len + STR_BUF; /*STR BUF for buffer*/
		str->cstr = (char*)_realloc(str->cstr, str->max, new_size);
		str->max = new_size;
	}

	strncpy(str->cstr, src, len);
	str->cstr[len] = 0;
	str->len = len;
	return str->cstr;
}

char* str_cpy(str_t* str, const char* src) {
	str_ncpy(str, src, 0x0FFFF);
	return str->cstr;
}

str_t* str_new(const char* s) {
	str_t* ret = (str_t*)_malloc(sizeof(str_t));
	ret->cstr = NULL;
	ret->max = 0;
	ret->len = 0;
	str_cpy(ret, s);
	return ret;
}

char* str_append(str_t* str, const char* src) {
	if(src == NULL || src[0] == 0) {
		return str->cstr;
	}

	int len = strlen(src);
	uint32_t new_size = str->len + len;
	if(str->max <= new_size) {
		new_size = str->len + len + STR_BUF; /*STR BUF for buffer*/
		str->cstr = (char*)_realloc(str->cstr, str->max, new_size);
		str->max = new_size;
	}

	strcpy(str->cstr + str->len, src);
	str->len = str->len + len;
	str->cstr[str->len] = 0;
	return str->cstr;
}

char* str_add(str_t* str, char c) {
	uint32_t new_size = str->len + 1;
	if(str->max <= new_size) {
		new_size = str->len + STR_BUF; /*STR BUF for buffer*/
		str->cstr = (char*)_realloc(str->cstr, str->max, new_size);
		str->max = new_size;
	}

	str->cstr[str->len] = c;
	str->len++;
	str->cstr[str->len] = 0;
	return str->cstr;
}

void str_free(str_t* str) {
	if(str == NULL)
		return;

	if(str->cstr != NULL) {
		_free(str->cstr);
	}
	_free(str);
}

const char* str_from_int(int i, char* s) {
	snprintf(s, STATIC_STR_MAX-1, "%d", i);
	return s;
}

const char* str_from_bool(bool b) {
	return b ? "true":"false";
}

const char* str_from_float(float i, char* s) {
	snprintf(s, STATIC_STR_MAX-1, "%f", i);
	return s;
}

int str_to_int(const char* str) {
	int i = 0;
	if(strstr(str, "0x") != NULL ||
			strstr(str, "0x") != NULL)
		i = strtol(str, NULL, 16);
	else
		i = strtol(str, NULL, 10);
	return i;
}

float str_to_float(const char* str) {
	return atof(str);
}

void str_split(const char* str, char c, m_array_t* array) {
	int i = 0;
	char offc = str[i];
	while(true) {
		if(offc == c || offc == 0) {
			char* p = (char*)_malloc(i+1);
			memcpy(p, str, i+1);
			p[i] = 0;
			array_add(array, p);
			if(offc == 0)
				break;

			str = str +  i + 1;
			i = 0;
			offc = str[i]; 
		}
		else {
			i++;
			offc = str[i]; 
		}
	}
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

