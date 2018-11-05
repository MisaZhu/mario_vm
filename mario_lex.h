#ifndef MARIO_LEX
#define MARIO_LEX

#include "mario_utils.h"

/** Script Lex. -----------------------------*/

#define  LEX_EOF  0

typedef struct st_lex {
	const char* data;

	int32_t data_pos;
	int32_t data_start, data_end;
	char curr_ch, next_ch;

	uint32_t tk;
	str_t* tk_str;
	int32_t tk_start, tk_end, tk_last_end;
} lex_t;

bool is_whitespace(unsigned char ch);

bool is_numeric(unsigned char ch);

bool is_number(const char* cstr);

bool is_hexadecimal(unsigned char ch);

bool is_alpha(unsigned char ch);

bool is_alpha_num(const char* cstr);

void lex_get_nextch(lex_t* lex);

void lex_reset(lex_t* lex);

void lex_init(lex_t * lex, const char* input);

void lex_release(lex_t* lex);

#endif
