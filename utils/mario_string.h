#ifndef MARIO_STRING
#define MARIO_STRING

#include "mario_utils.h"
#include "mario_array.h"

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif

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
char* str_add_int(str_t* str, int i, int base);
char* str_add_float(str_t* str, float f);
void str_free(str_t* str);
const char* str_from_int(int i, int base);
const char* str_from_float(float f);
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
