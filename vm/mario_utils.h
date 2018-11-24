/*
Some utils functions like str / array 
*/

#ifndef MARIO_UTILS
#define MARIO_UTILS

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

/**
array functions.
*/
typedef struct st_array {
	void** items;
	uint32_t max: 16;
	uint32_t size: 16;
} m_array_t;

m_array_t* array_new();
void array_free(m_array_t* array, free_func_t fr);
void array_init(m_array_t* array);
void array_add(m_array_t* array, void* item);
void array_add_head(m_array_t* array, void* item);
void* array_add_buf(m_array_t* array, void* s, uint32_t sz);
void* array_get(m_array_t* array, uint32_t index);
void* array_set(m_array_t* array, uint32_t index, void* p);
void* array_tail(m_array_t* array);
void* array_head(m_array_t* array);
void* array_remove(m_array_t* array, uint32_t index);
void array_del(m_array_t* array, uint32_t index, free_func_t fr);
void array_remove_all(m_array_t* array);
void array_clean(m_array_t* array, free_func_t fr);
#define array_tail(array) (((array)->items == NULL || (array)->size == 0) ? NULL: (array)->items[(array)->size-1]);

/**
string functions.
*/
typedef struct st_str {
	char* cstr;
	uint32_t max: 16;
	uint32_t len: 16;
} str_t;

void str_reset(str_t* str);
char* str_ncpy(str_t* str, const char* src, uint32_t l);
char* str_cpy(str_t* str, const char* src);
str_t* str_new(const char* s);
char* str_append(str_t* str, const char* src);
char* str_add(str_t* str, char c);
void str_free(str_t* str);
const char* str_from_int(int i, char* s);
const char* str_from_float(float i, char* s);
const char* str_from_bool(bool b);
int str_to_int(const char* str);
float str_to_float(const char* str);
void str_split(const char* str, char c, m_array_t* array);

/**
utf8 string functions
*/

typedef struct st_utf8_reader {
	const char* str;
	uint32_t offset;
} utf8_reader_t;

typedef m_array_t utf8_t;

void utf8_reader_init(utf8_reader_t* reader, const char* s, uint32_t offset);
bool utf8_read(utf8_reader_t* reader, str_t* dst);

utf8_t* utf8_new(const char* s);
void utf8_free(utf8_t* utf8);
void utf8_append_raw(utf8_t* utf8, const char* s);
void utf8_append(utf8_t* utf8, const char* s);
uint32_t utf8_len(utf8_t* utf8);
str_t* utf8_at(utf8_t* utf8, uint32_t at);
void utf8_set(utf8_t* utf8, uint32_t at, const char* s);
void utf8_to_str(utf8_t* utf8, str_t* str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
