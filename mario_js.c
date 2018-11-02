/**
very tiny js engine in single file.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "mario_js.h"
#include <stdio.h>

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
		array->items = (void**)_realloc(array->items, new_size*sizeof(void*)); 
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


#define array_tail(array) (((array)->items == NULL || (array)->size == 0) ? NULL: (array)->items[(array)->size-1]);

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
		str->cstr = (char*)_realloc(str->cstr, new_size);
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
		str->cstr = (char*)_realloc(str->cstr, new_size);
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
		str->cstr = (char*)_realloc(str->cstr, new_size);
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

#define FROM_STR_MAX 32
const char* str_from_int(int i, char* s) {
	snprintf(s, FROM_STR_MAX-1, "%d", i);
	return s;
}

const char* str_from_bool(bool b) {
	return b ? "true":"false";
}

const char* str_from_float(float i, char* s) {
	snprintf(s, FROM_STR_MAX-1, "%f", i);
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

/** Script Lex. -----------------------------*/

typedef enum {
  LEX_EOF = 0,
  LEX_ID = 256,
  LEX_INT,
  LEX_FLOAT,
  LEX_STR,

  LEX_EQUAL,
  LEX_TYPEEQUAL,
  LEX_NEQUAL,
  LEX_NTYPEEQUAL,
  LEX_LEQUAL,
  LEX_LSHIFT,
  LEX_LSHIFTEQUAL,
  LEX_GEQUAL,
  LEX_RSHIFT,
  LEX_RSHIFTUNSIGNED,
  LEX_RSHIFTEQUAL,
  LEX_PLUSEQUAL,
  LEX_MINUSEQUAL,
  LEX_MULTIEQUAL,
  LEX_DIVEQUAL,
  LEX_MODEQUAL,
  LEX_PLUSPLUS,
  LEX_MINUSMINUS,
  LEX_ANDEQUAL,
  LEX_ANDAND,
  LEX_OREQUAL,
  LEX_OROR,
  LEX_XOREQUAL,
  // reserved words
#define LEX_R_LIST_START LEX_R_IF
  LEX_R_IF,
  LEX_R_ELSE,
  LEX_R_DO,
  LEX_R_WHILE,
  LEX_R_FOR,
  LEX_R_BREAK,
  LEX_R_CONTINUE,
  LEX_R_STATIC,
  LEX_R_FUNCTION,
  LEX_R_CLASS,
  LEX_R_EXTENDS,
  LEX_R_RETURN,
  LEX_R_VAR,
  LEX_R_LET,
  LEX_R_CONST,
  LEX_R_TRUE,
  LEX_R_FALSE,
  LEX_R_NULL,
  LEX_R_UNDEFINED,
  LEX_R_NEW,
  LEX_R_TYPEOF,
  LEX_R_INCLUDE,
  LEX_R_THROW,
  LEX_R_TRY,
  LEX_R_CATCH,
  LEX_R_LIST_END /* always the last entry */
} LEX_TYPES;

typedef struct st_lex {
	const char* data;

	int32_t data_pos;
	int32_t data_start, data_end;
	char curr_ch, next_ch;

	LEX_TYPES tk;
	str_t* tk_str;
	int32_t tk_start, tk_end, tk_last_end;
} lex_t;

bool is_whitespace(unsigned char ch) {
	if(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
		return true;
	return false;
}

bool is_numeric(unsigned char ch) {
	if(ch >= '0' && ch <= '9')
		return true;
	return false;
}

bool is_number(const char* cstr) {
	int  i= 0;
	while(cstr[i] != 0) {
		if (is_numeric(cstr[i]) == false)
			return false;
		i++;
	}
	return true;
}

bool is_hexadecimal(unsigned char ch) {
	if(((ch>='0') && (ch<='9')) ||
		((ch>='a') && (ch<='f')) ||
		((ch>='A') && (ch<='F')))
			return true;
	return false;
}

bool is_alpha(unsigned char ch) {
	if(((ch>='a') && (ch<='z')) ||
		((ch>='A') && (ch<='Z')) ||
		ch == '_')
		return true;
	return false;
}

/** Is the std::string alphanumeric */
bool is_alpha_num(const char* cstr) {
	if (cstr[0] == 0){
		return true;
	}
	if (is_alpha(cstr[0]) == 0){
		return false;
	}

	int  i= 0;
	while(cstr[i] != 0) {
		if (is_alpha(cstr[i]) == false || is_numeric(cstr[i]) == true){
			return false;
		}
		i++;
	}
	return true;
}

void lex_get_nextch(lex_t* lex) {
	lex->curr_ch = lex->next_ch;
	if (lex->data_pos < lex->data_end){
		lex->next_ch = lex->data[lex->data_pos];
	}else{
		lex->next_ch = 0;
	}
	lex->data_pos++;
}

void lex_get_next_token(lex_t* lex) {
	lex->tk = LEX_EOF;
	str_reset(lex->tk_str);

	while (lex->curr_ch && is_whitespace(lex->curr_ch)){
		lex_get_nextch(lex);
	}
	// newline comments
	if ((lex->curr_ch=='/' && lex->next_ch=='/') || (lex->curr_ch=='#' && lex->next_ch=='!')) {
		while (lex->curr_ch && lex->curr_ch!='\n'){
			lex_get_nextch(lex);
		}
		lex_get_nextch(lex);
		lex_get_next_token(lex);
		return;
	}
	// block comments
	if (lex->curr_ch=='/' && lex->next_ch=='*') {
		while (lex->curr_ch && (lex->curr_ch!='*' || lex->next_ch!='/')) {
			lex_get_nextch(lex);
		}
		lex_get_nextch(lex);
		lex_get_nextch(lex);
		lex_get_next_token(lex);
		return;
	}
	// record beginning of this token(pre-read 2 chars );
	lex->tk_start = lex->data_pos-2;
	// tokens
	if (is_alpha(lex->curr_ch)) { //  IDs
		while (is_alpha(lex->curr_ch) || is_numeric(lex->curr_ch)) {
			str_add(lex->tk_str, lex->curr_ch);
			lex_get_nextch(lex);
		}
		lex->tk = LEX_ID;
		if (strcmp(lex->tk_str->cstr, "if") == 0)        lex->tk = LEX_R_IF;
		else if (strcmp(lex->tk_str->cstr, "else") == 0)      lex->tk = LEX_R_ELSE;
		else if (strcmp(lex->tk_str->cstr, "do") == 0)        lex->tk = LEX_R_DO;
		else if (strcmp(lex->tk_str->cstr, "while") == 0)    lex->tk = LEX_R_WHILE;
		else if (strcmp(lex->tk_str->cstr, "import") == 0)  lex->tk = LEX_R_INCLUDE;
		else if (strcmp(lex->tk_str->cstr, "for") == 0)     lex->tk = LEX_R_FOR;
		else if (strcmp(lex->tk_str->cstr, "break") == 0)    lex->tk = LEX_R_BREAK;
		else if (strcmp(lex->tk_str->cstr, "continue") == 0)  lex->tk = LEX_R_CONTINUE;
		else if (strcmp(lex->tk_str->cstr, "static") == 0)  lex->tk = LEX_R_STATIC;
		else if (strcmp(lex->tk_str->cstr, "function") == 0)  lex->tk = LEX_R_FUNCTION;
		else if (strcmp(lex->tk_str->cstr, "class") ==0) 		 lex->tk = LEX_R_CLASS;
		else if (strcmp(lex->tk_str->cstr, "extends") == 0) 	 lex->tk = LEX_R_EXTENDS;
		else if (strcmp(lex->tk_str->cstr, "return") == 0)   lex->tk = LEX_R_RETURN;
		else if (strcmp(lex->tk_str->cstr, "var")  == 0)      lex->tk = LEX_R_VAR;
		else if (strcmp(lex->tk_str->cstr, "let")  == 0)      lex->tk = LEX_R_LET;
		else if (strcmp(lex->tk_str->cstr, "const") == 0)     lex->tk = LEX_R_CONST;
		else if (strcmp(lex->tk_str->cstr, "true") == 0)      lex->tk = LEX_R_TRUE;
		else if (strcmp(lex->tk_str->cstr, "false") == 0)     lex->tk = LEX_R_FALSE;
		else if (strcmp(lex->tk_str->cstr, "null") == 0)      lex->tk = LEX_R_NULL;
		else if (strcmp(lex->tk_str->cstr, "undefined") == 0) lex->tk = LEX_R_UNDEFINED;
		else if (strcmp(lex->tk_str->cstr, "new") == 0)       lex->tk = LEX_R_NEW;
		else if (strcmp(lex->tk_str->cstr, "typeof") == 0)       lex->tk = LEX_R_TYPEOF;
		else if (strcmp(lex->tk_str->cstr, "throw") == 0)     lex->tk = LEX_R_THROW;
		else if (strcmp(lex->tk_str->cstr, "try") == 0)    	 lex->tk = LEX_R_TRY;
		else if (strcmp(lex->tk_str->cstr, "catch") == 0)     lex->tk = LEX_R_CATCH;
	} else if (is_numeric(lex->curr_ch)) { // _numbers
		bool isHex = false;
		if (lex->curr_ch=='0') {
			str_add(lex->tk_str, lex->curr_ch);
			lex_get_nextch(lex);
		}
		if (lex->curr_ch=='x') {
			isHex = true;
			str_add(lex->tk_str, lex->curr_ch);
			lex_get_nextch(lex);
		}
		lex->tk = LEX_INT;
		while (is_numeric(lex->curr_ch) || (isHex && is_hexadecimal(lex->curr_ch))) {
			str_add(lex->tk_str, lex->curr_ch);
			lex_get_nextch(lex);
		}
		if (!isHex && lex->curr_ch=='.') {
			lex->tk = LEX_FLOAT;
			str_add(lex->tk_str, '.');
			lex_get_nextch(lex);
			while (is_numeric(lex->curr_ch)) {
				str_add(lex->tk_str, lex->curr_ch);
				lex_get_nextch(lex);
			}
		}
		// do fancy e-style floating point
		if (!isHex && (lex->curr_ch=='e'||lex->curr_ch=='E')) {
			lex->tk = LEX_FLOAT;
			str_add(lex->tk_str, lex->curr_ch);
			lex_get_nextch(lex);
			if (lex->curr_ch=='-') {
				str_add(lex->tk_str, lex->curr_ch);
				lex_get_nextch(lex);
			}
			while (is_numeric(lex->curr_ch)) {
				str_add(lex->tk_str, lex->curr_ch);
				lex_get_nextch(lex);
			}
		}
	} else if (lex->curr_ch=='"') {
		// strings...
		lex_get_nextch(lex);
		while (lex->curr_ch && lex->curr_ch!='"') {
			if (lex->curr_ch == '\\') {
				lex_get_nextch(lex);
				switch (lex->curr_ch) {
					case 'n' : str_add(lex->tk_str, '\n'); break;
					case 'r' : str_add(lex->tk_str, '\r'); break;
					case 't' : str_add(lex->tk_str, '\t'); break;
					case '"' : str_add(lex->tk_str, '\"'); break;
					case '\\' : str_add(lex->tk_str, '\\'); break;
					default: str_add(lex->tk_str, lex->curr_ch);
				}
			} else {
				str_add(lex->tk_str, lex->curr_ch);
			}
			lex_get_nextch(lex);
		}
		lex_get_nextch(lex);
		lex->tk = LEX_STR;
	} else if (lex->curr_ch=='\'') {
		// strings again...
		lex_get_nextch(lex);
		while (lex->curr_ch && lex->curr_ch!='\'') {
			if (lex->curr_ch == '\\') {
				lex_get_nextch(lex);
				switch (lex->curr_ch) {
					case 'n' : str_add(lex->tk_str, '\n'); break;
					case 'a' : str_add(lex->tk_str, '\a'); break;
					case 'r' : str_add(lex->tk_str, '\r'); break;
					case 't' : str_add(lex->tk_str, '\t'); break;
					case '\'' : str_add(lex->tk_str, '\''); break;
					case '\\' : str_add(lex->tk_str, '\\'); break;
					case 'x' : { // hex digits
											 char buf[3] = "??";
											 lex_get_nextch(lex);
											 buf[0] = lex->curr_ch;
											 lex_get_nextch(lex);
											 buf[1] = lex->curr_ch;
											 str_add(lex->tk_str, (char)strtol(buf,0,16));
										 } break;
					default: if (lex->curr_ch>='0' && lex->curr_ch<='7') {
										 // octal digits
										 char buf[4] = "???";
										 buf[0] = lex->curr_ch;
										 lex_get_nextch(lex);
										 buf[1] = lex->curr_ch;
										 lex_get_nextch(lex);
										 buf[2] = lex->curr_ch;
										 str_add(lex->tk_str, (char)strtol(buf,0,8));
									 } else
										 str_add(lex->tk_str, lex->curr_ch);
				}
			} else {
				str_add(lex->tk_str, lex->curr_ch);
			}
			lex_get_nextch(lex);
		}
		lex_get_nextch(lex);
		lex->tk = LEX_STR;
	} else {
		// single chars
		lex->tk = (LEX_TYPES)lex->curr_ch;
		if (lex->curr_ch) 
			lex_get_nextch(lex);
		if (lex->tk=='=' && lex->curr_ch=='=') { // ==
			lex->tk = LEX_EQUAL;
			lex_get_nextch(lex);
			if (lex->curr_ch=='=') { // ===
				lex->tk = LEX_TYPEEQUAL;
				lex_get_nextch(lex);
			}
		} else if (lex->tk=='!' && lex->curr_ch=='=') { // !=
			lex->tk = LEX_NEQUAL;
			lex_get_nextch(lex);
			if (lex->curr_ch=='=') { // !==
				lex->tk = LEX_NTYPEEQUAL;
				lex_get_nextch(lex);
			}
		} else if (lex->tk=='<' && lex->curr_ch=='=') {
			lex->tk = LEX_LEQUAL;
			lex_get_nextch(lex);
		} else if (lex->tk=='<' && lex->curr_ch=='<') {
			lex->tk = LEX_LSHIFT;
			lex_get_nextch(lex);
			if (lex->curr_ch=='=') { // <<=
				lex->tk = LEX_LSHIFTEQUAL;
				lex_get_nextch(lex);
			}
		} else if (lex->tk=='>' && lex->curr_ch=='=') {
			lex->tk = LEX_GEQUAL;
			lex_get_nextch(lex);
		} else if (lex->tk=='>' && lex->curr_ch=='>') {
			lex->tk = LEX_RSHIFT;
			lex_get_nextch(lex);
			if (lex->curr_ch=='=') { // >>=
				lex->tk = LEX_RSHIFTEQUAL;
				lex_get_nextch(lex);
			} else if (lex->curr_ch=='>') { // >>>
				lex->tk = LEX_RSHIFTUNSIGNED;
				lex_get_nextch(lex);
			}
		}  else if (lex->tk=='+' && lex->curr_ch=='=') {
			lex->tk = LEX_PLUSEQUAL;
			lex_get_nextch(lex);
		}  else if (lex->tk=='-' && lex->curr_ch=='=') {
			lex->tk = LEX_MINUSEQUAL;
			lex_get_nextch(lex);
		}  else if (lex->tk=='*' && lex->curr_ch=='=') {
			lex->tk = LEX_MULTIEQUAL;
			lex_get_nextch(lex);
		}  else if (lex->tk=='/' && lex->curr_ch=='=') {
			lex->tk = LEX_DIVEQUAL;
			lex_get_nextch(lex);
		}  else if (lex->tk=='%' && lex->curr_ch=='=') {
			lex->tk = LEX_MODEQUAL;
			lex_get_nextch(lex);
		}  else if (lex->tk=='+' && lex->curr_ch=='+') {
			lex->tk = LEX_PLUSPLUS;
			lex_get_nextch(lex);
		}  else if (lex->tk=='-' && lex->curr_ch=='-') {
			lex->tk = LEX_MINUSMINUS;
			lex_get_nextch(lex);
		} else if (lex->tk=='&' && lex->curr_ch=='=') {
			lex->tk = LEX_ANDEQUAL;
			lex_get_nextch(lex);
		} else if (lex->tk=='&' && lex->curr_ch=='&') {
			lex->tk = LEX_ANDAND;
			lex_get_nextch(lex);
		} else if (lex->tk=='|' && lex->curr_ch=='=') {
			lex->tk = LEX_OREQUAL;
			lex_get_nextch(lex);
		} else if (lex->tk=='|' && lex->curr_ch=='|') {
			lex->tk = LEX_OROR;
			lex_get_nextch(lex);
		} else if (lex->tk=='^' && lex->curr_ch=='=') {
			lex->tk = LEX_XOREQUAL;
			lex_get_nextch(lex);
		}
	}
	/* This isn't quite right yet */
	lex->tk_last_end = lex->tk_end;
	lex->tk_end = lex->data_pos-3;
}

void lex_reset(lex_t* lex) {
	lex->data_pos = lex->data_start;
	lex->tk_start   = 0;
	lex->tk_end     = 0;
	lex->tk_last_end = 0;
	lex->tk  = LEX_EOF;
	str_reset(lex->tk_str);
	lex_get_nextch(lex);
	lex_get_nextch(lex);
	lex_get_next_token(lex);
}

void lex_init(lex_t * lex, const char* input) {
	lex->data = input;
	lex->data_start = 0;
	lex->data_end = strlen(lex->data);
	lex->tk_str = str_new("");
	lex_reset(lex);
}

void lex_release(lex_t* lex) {
	str_free(lex->tk_str);
	lex->tk_str = NULL;
}

#ifdef MARIO_DEBUG

const char* lex_get_token_str(int token) {
	if (token>32 && token<128) {
		static char buf[4] = "' '";
		buf[1] = (char)token;
		return buf;
	}
	switch (token) {
		case LEX_EOF            : return "EOF";
		case LEX_ID             : return "ID";
		case LEX_INT            : return "INT";
		case LEX_FLOAT          : return "FLOAT";
		case LEX_STR            : return "STRING";
		case LEX_EQUAL          : return "==";
		case LEX_TYPEEQUAL      : return "===";
		case LEX_NEQUAL         : return "!=";
		case LEX_NTYPEEQUAL     : return "!==";
		case LEX_LEQUAL         : return "<=";
		case LEX_LSHIFT         : return "<<";
		case LEX_LSHIFTEQUAL    : return "<<=";
		case LEX_GEQUAL         : return ">=";
		case LEX_RSHIFT         : return ">>";
		case LEX_RSHIFTUNSIGNED : return ">>";
		case LEX_RSHIFTEQUAL    : return ">>=";
		case LEX_PLUSEQUAL      : return "+=";
		case LEX_MINUSEQUAL     : return "-=";
		case LEX_MULTIEQUAL     : return "*=";
		case LEX_DIVEQUAL 	    : return "/=";
		case LEX_MODEQUAL   	  : return "%=";
		case LEX_PLUSPLUS       : return "++";
		case LEX_MINUSMINUS     : return "--";
		case LEX_ANDEQUAL       : return "&=";
		case LEX_ANDAND         : return "&&";
		case LEX_OREQUAL        : return "|=";
		case LEX_OROR           : return "||";
		case LEX_XOREQUAL       : return "^=";
															// reserved words
		case LEX_R_IF           : return "if";
		case LEX_R_ELSE         : return "else";
		case LEX_R_DO           : return "do";
		case LEX_R_WHILE        : return "while";
		case LEX_R_FOR          : return "for";
		case LEX_R_BREAK        : return "break";
		case LEX_R_CONTINUE     : return "continue";
		case LEX_R_STATIC       : return "static";
		case LEX_R_FUNCTION     : return "function";
		case LEX_R_CLASS     		: return "class";
		case LEX_R_EXTENDS   		: return "extends";
		case LEX_R_RETURN       : return "return";
		case LEX_R_CONST        : return "CONST";
		case LEX_R_VAR          : return "var";
		case LEX_R_LET          : return "let";
		case LEX_R_TRUE         : return "true";
		case LEX_R_FALSE        : return "false";
		case LEX_R_NULL         : return "null";
		case LEX_R_UNDEFINED    : return "undefined";
		case LEX_R_NEW          : return "new";
		case LEX_R_INCLUDE      : return "import";
	}
	return "?[UNKNOW]";
}

#endif

void lex_get_pos(lex_t* lex, int* line, int *col, int pos) {
	if (pos<0) 
		pos= lex->tk_last_end;

	int l = 1;
	int c  = 1;
	int i;
	for (i=0; i<pos; i++) {
		char ch;
		if (i < lex->data_end){
			ch = lex->data[i];
		}else{
			ch = 0;
		}

		c++;
		if (ch=='\n') {
			l++;
			c = 1;
		}
	}
	*line = l;
	*col = c;
}

void lex_get_pos_str(lex_t* l, int pos, str_t* ret) {
	int line = 1;
	int col;

	char s[FROM_STR_MAX];
	lex_get_pos(l, &line, &col, pos);
	str_reset(ret);
	str_append(ret, "(line: ");
	str_append(ret, str_from_int(line, s));
	str_append(ret, ", col: ");
	str_append(ret, str_from_int(col, s));
	str_append(ret, ")");
}

bool lex_chkread(lex_t* lex, uint32_t expected_tk) {
	if (lex->tk != expected_tk) {
#ifdef MARIO_DEBUG
		_err("Got ");
		_err(lex_get_token_str(lex->tk));
		_err(" expected ");
		_err(lex_get_token_str(expected_tk));
#endif
		str_t* s = str_new("");
		lex_get_pos_str(lex, -1, s);
		_err(s->cstr);
		str_free(s);
		_err("!\n");
		return false;
	}
	lex_get_next_token(lex);
	return true;
}



/** JS bytecode.-----------------------------*/

typedef uint16_t opr_code_t;

#define ILLEGAL_PC 0x0FFFFFFF
#define INSTR_NEED_IMPROVE	 0x80000000 // NEED CACHE next time.
#define INS(ins, off) (((((int32_t)ins) << 16) & 0xFFFF0000) | ((off) & 0x0000FFFF))
#define OP(ins) (((ins) >>16) & 0x0FFF)

#define INSTR_NIL					 0x0000 // NIL									: Do nothing.

#define INSTR_VAR					 0x0001 // VAR x								: declare var x
#define INSTR_CONST				 0x0002 // CONST x							: declare const x
#define INSTR_LOAD				 0x0003 // LOAD x								: load and push x 
#define INSTR_STORE				 0x0004 // STORE x							: pop and store to x
#define INSTR_GET					 0x0005 // getfield
#define INSTR_ASIGN				 0x0006 // ASIGN								: =

#define INSTR_INT					 0x0007 // INT int 							: push int
#define INSTR_FLOAT				 0x0008 // FLOAT float					: push float 
#define INSTR_STR					 0x0009 // STR "str"						: push str
#define INSTR_ARRAY_AT		 0x000A // ARRAT 								: get array element at
#define INSTR_ARRAY				 0x000B // ARRAY 								: array start
#define INSTR_ARRAY_END		 0x000C // ARRAY_END 						: array end
#define INSTR_INT_S				 0x000D // SHORT_INT int 				: push short int
#define INSTR_LET					 0x000E // LET x								: declare let x
#define INSTR_CACHE				 0x000F // CACHE index					: load cache at 'index' and push 

#define INSTR_FUNC				 0x0010 // FUNC x								: function definetion x
#define INSTR_FUNC_GET		 0x0011 // GET FUNC x						: class get function definetion x
#define INSTR_FUNC_SET		 0x0012 // SET FUNC x						: class set function definetion x
#define INSTR_CALL				 0x0013 // CALL x								: call function x and push res
#define INSTR_CALLO				 0x0014 // CALL obj.x						: call object member function x and push res
#define INSTR_CLASS				 0x0015 // class								: define class
#define INSTR_CLASS_END		 0x0016 // class end						: end of class definition
#define INSTR_MEMBER			 0x0017 // member without name
#define INSTR_MEMBERN			 0x0018 // : member with name
#define INSTR_EXTENDS			 0x0019 // : class extends
#define INSTR_FUNC_STC		 0x001A // ST FUNC x						: static function definetion x

#define INSTR_NOT					 0x0020 // NOT									: !
#define INSTR_MULTI				 0x0021 // MULTI								: *
#define INSTR_DIV					 0x0022 // DIV									: /
#define INSTR_MOD					 0x0023 // MOD									: %
#define INSTR_PLUS				 0x0024 // PLUS									: + 
#define INSTR_MINUS				 0x0025 // MINUS								: - 
#define INSTR_NEG					 0x0026 // NEG									: negate -
#define INSTR_PPLUS				 0x0027 // PPLUS								: x++
#define INSTR_MMINUS			 0x0028 // MMINUS								: x--
#define INSTR_PPLUS_PRE		 0x0029 // PPLUS								: ++x
#define INSTR_MMINUS_PRE	 0x002A // MMINUS								: --x
#define INSTR_LSHIFT			 0x002B // LSHIFT								: <<
#define INSTR_RSHIFT			 0x002C // RSHIFT								: >>
#define INSTR_URSHIFT			 0x002D // URSHIFT							: >>>

#define INSTR_EQ					 0x0030 // EQ										: ==
#define INSTR_NEQ					 0x0031 // NEQ									: !=
#define INSTR_LEQ					 0x0032 // LEQ									: <=
#define INSTR_GEQ					 0x0033 // GEQ									: >=
#define INSTR_GRT					 0x0034 // GRT									: >
#define INSTR_LES					 0x0035 // LES									: <

#define INSTR_PLUSEQ			 0x0036 // +=
#define INSTR_MINUSEQ			 0x0037 // -=	
#define INSTR_MULTIEQ			 0x0038 // *=
#define INSTR_DIVEQ				 0x0039 // /=
#define INSTR_MODEQ				 0x003A // %=

#define INSTR_AAND				 0x0040 // AAND									: &&
#define INSTR_OOR					 0x0041 // OOR									: ||
#define INSTR_OR					 0x0042 // OR										: |
#define INSTR_XOR					 0x0043 // XOR									: ^
#define INSTR_AND					 0x0044 // AND									: &

#define INSTR_TEQ					 0x0046 // TEQ										: ===
#define INSTR_NTEQ				 0x0047 // NTEQ									: !==
#define INSTR_TYPEOF			0x0048 // TYPEOF									: typeof

#define INSTR_BREAK				 0x0050 // : break
#define INSTR_CONTINUE		 0x0051 // : continue
#define INSTR_RETURN			 0x0052 // : return none value
#define INSTR_RETURNV			 0x0053 // : return with value

#define INSTR_NJMP				 0x0054 // NJMP x								: Condition not JMP offset x 
#define INSTR_JMPB				 0x0055 // JMP back x						: JMP back offset x  
#define INSTR_NJMPB				 0x0056 // NJMP back x					: Condition not JMP back offset x 
#define INSTR_JMP					 0x0057 // JMP x								: JMP offset x  

#define INSTR_TRUE				 0x0060 // : true
#define INSTR_FALSE				 0x0061 // : false
#define INSTR_NULL				 0x0062 // : null
#define INSTR_UNDEF				 0x0063 // : undefined

#define INSTR_NEW					 0x0070 // : new

#define INSTR_POP					 0x0080 // : pop and release

#define INSTR_OBJ					 0x0090 // : object for JSON 
#define INSTR_OBJ_END			 0x0091 // : object end for JSON 

#define INSTR_BLOCK				 0x00A0 // : block 
#define INSTR_BLOCK_END		 0x00A1 // : block end 

#define INSTR_THROW				 0x00B0 // : throw
#define INSTR_MOV_EXCP		 0x00B1 // : move exception

#define INSTR_END					 0x00FF // END									: end of code.

#define BC_BUF_SIZE  3232


uint16_t bc_getstrindex(bytecode_t* bc, const char* str) {
	uint16_t sz = bc->str_table.size;
	uint16_t i;
	if(str == NULL || str[0] == 0)
		return 0xFFFF;

	for(i=0; i<sz; ++i) {
		char* s = (char*)bc->str_table.items[i];
		if(s != NULL && strcmp(s, str) == 0)
			return i;
	}

	uint32_t len = strlen(str);
	char* p = (char*)_malloc(len + 1);
	memcpy(p, str, len+1);
	array_add(&bc->str_table, p);
	return sz;
}	

void bc_init(vm_t* vm, bytecode_t* bc) {
	bc->cindex = 0;
	bc->code_buf = NULL;
	bc->buf_size = 0;
	array_init(&bc->str_table);

	vm->this_strIndex = bc_getstrindex(bc, THIS);
}

void bc_release(bytecode_t* bc) {
	array_clean(&bc->str_table, NULL, NULL);
	if(bc->code_buf != NULL)
		_free(bc->code_buf);
}

void bc_add(bytecode_t* bc, PC ins) {
	if(bc->cindex >= bc->buf_size) {
		bc->buf_size = bc->cindex + BC_BUF_SIZE;
		PC *new_buf = (PC*)_malloc(bc->buf_size*sizeof(PC));

		if(bc->cindex > 0 && bc->code_buf != NULL) {
			memcpy(new_buf, bc->code_buf, bc->cindex*sizeof(PC));
			_free(bc->code_buf);
		}
		bc->code_buf = new_buf;
	}

	bc->code_buf[bc->cindex] = ins;
	bc->cindex++;
}
	
PC bc_reserve(bytecode_t* bc) {
	bc_add(bc, INS(INSTR_NIL, 0xFFFF));
  return bc->cindex-1;
}

#define bc_getstr(bc, i) ((i>=(bc)->str_table.size) ? "" : (const char*)(bc)->str_table.items[i])

/*const char* bc_getstr(bytecode_t* bc, int i) {
	if(i<0 || i == 0xFFFF ||  i>=bc->str_table.size)
		return "";
	return (const char*)bc->str_table.items[i];
}	
*/

PC bc_bytecode(bytecode_t* bc, opr_code_t instr, const char* str) {
	opr_code_t r = instr;
	opr_code_t i = 0xFFFF;

	if(str != NULL && str[0] != 0)
		i = bc_getstrindex(bc, str);

	return INS(r, i);
}
	
PC bc_gen_int(bytecode_t* bc, opr_code_t instr, int32_t i) {
	PC ins = bc_bytecode(bc, instr, "");
	bc_add(bc, ins);
	bc_add(bc, i);
	return bc->cindex;
}

PC bc_gen_short(bytecode_t* bc, opr_code_t instr, int32_t s) {
	PC ins = bc_bytecode(bc, instr, "");
	ins = (ins&0xFFFF0000) | (s&0x0FFFF);
	bc_add(bc, ins);
	return bc->cindex;
}
	
PC bc_gen_str(bytecode_t* bc, opr_code_t instr, const char* str) {
	uint32_t i = 0;
	float f = 0.0;
	const char* s = str;

	if(instr == INSTR_INT) {
		if(strstr(str, "0x") != NULL ||
				strstr(str, "0x") != NULL)
			i = strtol(str, NULL, 16);
		else
			i = strtol(str, NULL, 10);
		s = NULL;
	}
	else if(instr == INSTR_FLOAT) {
		f = atof(str);
		s = NULL;
	}
	
	PC ins = bc_bytecode(bc, instr, s);
	bc_add(bc, ins);

	if(instr == INSTR_INT) {
		if(i < 0xFFFF) //short int
			bc->code_buf[bc->cindex-1] = INS(INSTR_INT_S, i);
		else 	
			bc_add(bc, i);
	}
	else if(instr == INSTR_FLOAT) {
		memcpy(&i, &f, sizeof(PC));
		bc_add(bc, i);
	}
	return bc->cindex;
}

PC bc_gen(bytecode_t* bc, opr_code_t instr) {
	return bc_gen_str(bc, instr, "");
}

void bc_set_instr(bytecode_t* bc, PC anchor, opr_code_t op, PC target) {
	if(target == ILLEGAL_PC)
		target = bc->cindex;

	int offset = target > anchor ? (target-anchor) : (anchor-target);
	PC ins = INS(op, offset);
	bc->code_buf[anchor] = ins;
}

PC bc_add_instr(bytecode_t* bc, PC anchor, opr_code_t op, PC target) {
	if(target == ILLEGAL_PC)
		target = bc->cindex;

	int offset = target > anchor ? (target-anchor) : (anchor-target);
	PC ins = INS(op, offset);
	bc_add(bc, ins);
	return bc->cindex;
} 

#ifdef MARIO_DEBUG

const char* instr_str(opr_code_t ins) {
	switch(ins) {
		case  INSTR_NIL					: return "NIL";
		case  INSTR_END					: return "END";
		case  INSTR_OBJ					: return "OBJ";
		case  INSTR_OBJ_END			: return "OBJE";
		case  INSTR_MEMBER			: return "MEMBER";
		case  INSTR_MEMBERN			: return "MEMBERN";
		case  INSTR_POP					: return "POP";
		case  INSTR_VAR					: return "VAR";
		case  INSTR_LET					: return "LET";
		case  INSTR_CONST				: return "CONST";
		case  INSTR_INT					: return "INT";
		case  INSTR_INT_S				: return "INTS";
		case  INSTR_FLOAT				: return "FLOAT";
		case  INSTR_STR					: return "STR";
		case  INSTR_ARRAY_AT		: return "ARRAT";
		case  INSTR_ARRAY				: return "ARR";
		case  INSTR_ARRAY_END		: return "ARRE";
		case  INSTR_LOAD				: return "LOAD";
		case  INSTR_STORE				: return "STORE";
		case  INSTR_JMP					: return "JMP";
		case  INSTR_NJMP				: return "NJMP";
		case  INSTR_JMPB				: return "JMPB";
		case  INSTR_NJMPB				: return "NJMPB";
		case  INSTR_FUNC				: return "FUNC";
		case  INSTR_FUNC_STC		: return "FUNC_STC";
		case  INSTR_FUNC_GET		: return "FUNCGET";
		case  INSTR_FUNC_SET		: return "FUNCSET";
		case  INSTR_CLASS				: return "CLASS";
		case  INSTR_CLASS_END		: return "CLASSE";
		case  INSTR_EXTENDS			: return "EXTENDS";
		case  INSTR_CALL				: return "CALL";
		case  INSTR_CALLO				: return "CALLO";
		case  INSTR_NOT					: return "NOT";
		case  INSTR_MULTI				: return "MULTI";
		case  INSTR_DIV					: return "DIV";
		case  INSTR_MOD					: return "MOD";
		case  INSTR_PLUS				: return "PLUS";
		case  INSTR_MINUS				: return "MINUS";
		case  INSTR_NEG					: return "NEG";
		case  INSTR_PPLUS				: return "PPLUS";
		case  INSTR_MMINUS			: return "MMINUS";
		case  INSTR_PPLUS_PRE		: return "PPLUSP";
		case  INSTR_MMINUS_PRE	: return "MMINUSP";
		case  INSTR_LSHIFT			: return "LSHIFT";
		case  INSTR_RSHIFT			: return "RSHIFT";
		case  INSTR_URSHIFT			: return "URSHIFT";
		case  INSTR_EQ					: return "EQ";
		case  INSTR_NEQ					: return "NEQ";
		case  INSTR_LEQ					: return "LEQ";
		case  INSTR_GEQ					: return "GEQ";
		case  INSTR_GRT					: return "GRT";
		case  INSTR_LES					: return "LES";
		case  INSTR_PLUSEQ			: return "PLUSEQ";
		case  INSTR_MINUSEQ			: return "MINUSEQ";
		case  INSTR_MULTIEQ			: return "MULTIEQ";
		case  INSTR_DIVEQ				: return "DIVEQ";
		case  INSTR_MODEQ				: return "MODEQ";
		case  INSTR_AAND				: return "AAND";
		case  INSTR_OOR					: return "OOR";
		case  INSTR_OR					: return "OR";
		case  INSTR_XOR					: return "XOR";
		case  INSTR_AND					: return "AND";
		case  INSTR_ASIGN				: return "ASIGN";
		case  INSTR_BREAK				: return "BREAK";
		case  INSTR_CONTINUE		: return "CONTINUE";
		case  INSTR_RETURN			: return "RETURN";
		case  INSTR_RETURNV			: return "RETURNV";
		case  INSTR_TRUE				: return "TRUE";
		case  INSTR_FALSE				: return "FALSE";
		case  INSTR_NULL				: return "NULL";
		case  INSTR_UNDEF				: return "UNDEF";
		case  INSTR_NEW					: return "NEW";
		case  INSTR_GET					: return "GET";
		case  INSTR_BLOCK				: return "BLOCK";
		case  INSTR_BLOCK_END		: return "BLOCKE";
		case  INSTR_THROW				: return "THROW";
		case  INSTR_MOV_EXCP		: return "MOVEXCP";
		default									: return "";
	}
}

PC bc_get_instr_str(bytecode_t* bc, PC i, str_t* ret) {
	PC ins = bc->code_buf[i];
	opr_code_t instr = (ins >> 16) & 0xFFFF;
	opr_code_t offset = ins & 0xFFFF;

	char s[64];
	str_reset(ret);

	if(offset == 0xFFFF) {
		sprintf(s, "  |%04d 0x%08X ; %s", i, ins, instr_str(instr));	
		str_append(ret, s);
	}
	else {
		if(instr == INSTR_JMP || 
				instr == INSTR_NJMP || 
				instr == INSTR_NJMPB ||
				instr == INSTR_JMPB ||
				instr == INSTR_INT_S ||
				instr == INSTR_BLOCK_END) {
			sprintf(s, "  |%04d 0x%08X ; %s\t%d", i, ins, instr_str(instr), offset);	
			str_append(ret, s);
		}
		else {
			sprintf(s, "  |%04d 0x%08X ; %s\t\"", i, ins, instr_str(instr));	
			str_append(ret, s);
			str_append(ret, bc_getstr(bc, offset));
			str_add(ret, '"');
		}
	}
	
	if(instr == INSTR_INT) {
		ins = bc->code_buf[i+1];
		sprintf(s, "\n  |%04d 0x%08X ; (%d)", i+1, ins, ins);	
		str_append(ret, s);
		i++;
	}
	else if(instr == INSTR_FLOAT) {
		ins = bc->code_buf[i+1];
		float f;
		memcpy(&f, &ins, sizeof(PC));
		sprintf(s, "\n  |%04d 0x%08X ; (%f)", i+1, ins, f);	
		str_append(ret, s);
		i++;
	}	
	return i;
}

void bc_dump(bytecode_t* bc) {
	PC i;
	char index[8];
	PC sz = bc->str_table.size;

	_out_func("-------string table--------------------\n");
	for(i=0; i<sz; ++i) {
		sprintf(index, "%04X: ", i);
		_out_func(index);
		_out_func((const char*)bc->str_table.items[i]);
		_out_func("\n");
	}
	_out_func("---------------------------------------\n");

	str_t* s = str_new("");

	i = 0;
	while(i < bc->cindex) {
		i = bc_get_instr_str(bc, i, s);
		_out_func(s->cstr);
		_out_func("\n");
		i++;
	}
	str_free(s);
	_out_func("---------------------------------------\n");
}

#endif

/** Compiler -----------------------------*/

typedef struct st_loop {
	PC continueAnchor;
	PC breakAnchor;
	uint32_t blockDepth;
} loop_t;

bool statement(lex_t*, bytecode_t*, bool, loop_t*);
bool factor(lex_t*, bytecode_t*);
bool base(lex_t*, bytecode_t*);

void gen_func_name(const char* name, int arg_num, str_t* full) {
	str_reset(full);
	str_cpy(full, name);
	if(arg_num > 0) {
		str_append(full, "$");
		char s[FROM_STR_MAX];
		str_append(full, str_from_int(arg_num, s));
	}
}

int parse_func_name(const char* full, str_t* name) {
	const char* pos = strchr(full, '$');
	int args_num = 0;
	if(pos != NULL) {
		args_num = atoi(pos+1);
		if(name != NULL)
			str_ncpy(name, full, pos-full);	
	}
	else {
		if(name != NULL)
			str_cpy(name, full);
	}
	return args_num;
}

int callFunc(lex_t* l, bytecode_t* bc) {
	if(!lex_chkread(l, '(')) return -1;
	int arg_num = 0;
	while(true) {
		PC pc1 = bc->cindex;
		if(!base(l, bc))
			return -1;
		PC pc2 = bc->cindex;
		if(pc2 > pc1) //not empty, means valid arguemnt.
			arg_num++;

		if (l->tk!=')') {
			if(!lex_chkread(l, ',')) return -1;	
		}
		else
			break;
	}
	if(!lex_chkread(l, ')')) return -1;
	return arg_num;
}

bool block(lex_t* l, bytecode_t* bc, loop_t* loop, bool func) {
	bool doBlock = false;
	if(!lex_chkread(l, '{')) return false;

	if(!func) 
		doBlock = true;

	if(loop) {
		if(loop->blockDepth == 0)
			doBlock = false;
		loop->blockDepth++;
	}
	
	if(doBlock)
		bc_gen(bc, INSTR_BLOCK);

	while (l->tk && l->tk!='}'){
		if(!statement(l, bc, true, loop))
			return false;
	}
	if(!lex_chkread(l, '}')) return false;


	if(loop)
		loop->blockDepth--;

	if(doBlock) 
		bc_gen_short(bc, INSTR_BLOCK_END, 1);
	return true;
}

bool defFunc(lex_t* l, bytecode_t* bc, str_t* name) {
	bool is_static = false;
	if (l->tk == LEX_R_STATIC) {
		if(!lex_chkread(l, LEX_R_STATIC)) return false;
		is_static = true;
	}

	/* we can have functions without names */
	if (l->tk == LEX_ID) {
		str_cpy(name, l->tk_str->cstr);
		if(!lex_chkread(l, LEX_ID)) return false;
	}
	
	if(l->tk == LEX_ID) { //class get/set token
		if(strcmp(name->cstr, "get") == 0) {
			str_cpy(name, l->tk_str->cstr);
			if(!lex_chkread(l, LEX_ID)) return false;
			bc_gen(bc, INSTR_FUNC_GET);
		}
		if(strcmp(name->cstr, "set") == 0) {
			str_cpy(name, l->tk_str->cstr);
			if(!lex_chkread(l, LEX_ID)) return false;
			bc_gen(bc, INSTR_FUNC_SET);
		}
	}
	else {
		bc_gen(bc, is_static ? INSTR_FUNC_STC:INSTR_FUNC);
	}
	//do arguments
	if(!lex_chkread(l, '(')) return false;
	while (l->tk!=')') {
		bc_gen_str(bc, INSTR_VAR, l->tk_str->cstr);
		if(!lex_chkread(l, LEX_ID)) return false;
		if (l->tk!=')') {
			if(!lex_chkread(l, ',')) return false;
		}
	}
	if(!lex_chkread(l, ')')) return false;
	PC pc = bc_reserve(bc);
	block(l, bc, NULL, true);
	
	opr_code_t op = bc->code_buf[bc->cindex - 1] >> 16;

	if(op != INSTR_RETURN && op != INSTR_RETURNV)
		bc_gen(bc, INSTR_RETURN);
	bc_set_instr(bc, pc, INSTR_JMP, ILLEGAL_PC);
	return true;
}

bool def_class(lex_t* l, bytecode_t* bc) {
	// actually parse a class...
	if(!lex_chkread(l, LEX_R_CLASS)) return false;
	str_t* name = str_new("");

	/* we can have classes without names */
	if (l->tk==LEX_ID) {
		str_cpy(name, l->tk_str->cstr);
		if(!lex_chkread(l, LEX_ID)) return false;
	}
	bc_gen_str(bc, INSTR_CLASS, name->cstr);
	
	/*read extends*/
	if (l->tk==LEX_R_EXTENDS) {
		if(!lex_chkread(l, LEX_R_EXTENDS)) return false;
		str_cpy(name, l->tk_str->cstr);
		if(!lex_chkread(l, LEX_ID)) return false;
		bc_gen_str(bc, INSTR_EXTENDS, name->cstr);
	}

	if(!lex_chkread(l, '{')) return false;
	while (l->tk!='}') {
		if(!defFunc(l, bc, name))
			return false;
		bc_gen_str(bc, INSTR_MEMBERN, name->cstr);
	}
	if(!lex_chkread(l, '}')) return false;
	bc_gen(bc, INSTR_CLASS_END);

	str_free(name);
	return true;
}


bool factor(lex_t* l, bytecode_t* bc) {
	if (l->tk=='(') {
		if(!lex_chkread(l, '(')) return false;
		if(!base(l, bc)) return false;
		if(!lex_chkread(l, ')')) return false;
	}
	else if (l->tk==LEX_R_TRUE) {
		if(!lex_chkread(l, LEX_R_TRUE)) return false;
		bc_gen(bc, INSTR_TRUE);
	}
	else if (l->tk==LEX_R_FALSE) {
		if(!lex_chkread(l, LEX_R_FALSE)) return false;
		bc_gen(bc, INSTR_FALSE);
	}
	else if (l->tk==LEX_R_NULL) {
		if(!lex_chkread(l, LEX_R_NULL)) return false;
		bc_gen(bc, INSTR_NULL);
	}
	else if (l->tk==LEX_R_UNDEFINED) {
		if(!lex_chkread(l, LEX_R_UNDEFINED)) return false;
		bc_gen(bc, INSTR_UNDEF);
	}
	else if (l->tk==LEX_INT) {
		bc_gen_str(bc, INSTR_INT, l->tk_str->cstr);
		if(!lex_chkread(l, LEX_INT)) return false;
	}
	else if (l->tk==LEX_FLOAT) {
		bc_gen_str(bc, INSTR_FLOAT, l->tk_str->cstr);
		if(!lex_chkread(l, LEX_FLOAT)) return false;
	}
	else if (l->tk==LEX_STR) {
		bc_gen_str(bc, INSTR_STR, l->tk_str->cstr);
		if(!lex_chkread(l, LEX_STR)) return false;
	}
	else if(l->tk==LEX_R_FUNCTION) {
		if(!lex_chkread(l, LEX_R_FUNCTION)) return false;
		str_t *fname = str_new("");
		defFunc(l, bc, fname);
		str_free(fname);
	}
	else if(l->tk==LEX_R_CLASS) {
		def_class(l, bc);
	}
	else if (l->tk==LEX_R_NEW) {
		// new -> create a new object
		if(!lex_chkread(l, LEX_R_NEW)) return false;
		str_t* class_name = str_new("");
		str_cpy(class_name, l->tk_str->cstr);

		if(!lex_chkread(l, LEX_ID)) return false;
		if (l->tk == '(') {
			//lex_chkread(l, '(');
			int arg_num = callFunc(l, bc);
			//lex_chkread(l, ')');
			if(arg_num > 0) {
				str_append(class_name, "$");
				char s[FROM_STR_MAX];
				str_append(class_name, str_from_int(arg_num, s));
			}
			bc_gen_str(bc, INSTR_NEW, class_name->cstr);
		}
		str_free(class_name);
	}

	if (l->tk=='{') {
		// JSON-style object definition
		if(!lex_chkread(l, '{')) return false;
		bc_gen(bc, INSTR_OBJ);
		while (l->tk != '}') {
			str_t* id = str_new(l->tk_str->cstr);
			// we only allow strings or IDs on the left hand side of an initialisation
			if (l->tk==LEX_STR) {
				if(!lex_chkread(l, LEX_STR)) return false;
			}
			else {
				if(!lex_chkread(l, LEX_ID)) return false;
			}	

			if(!lex_chkread(l, ':')) return false;
			if(!base(l, bc)) return false;

			bc_gen_str(bc, INSTR_MEMBERN, id->cstr);
			// no need to clean here, as it will definitely be used
			if (l->tk != '}') {
				if(!lex_chkread(l, ',')) return false;
			}
			str_free(id);
		}
		bc_gen(bc, INSTR_OBJ_END);
		if(!lex_chkread(l, '}')) return false;
	}
	else if(l->tk==LEX_ID) {
		str_t* name = str_new(l->tk_str->cstr);
		if(!lex_chkread(l, LEX_ID)) return false;

		m_array_t names;
		array_init(&names);

		bool load = true;
		while (l->tk=='(' || l->tk=='.' || l->tk=='[') {
			if (l->tk=='(') { // ------------------------------------- Function Call
				str_split(name->cstr, '.', &names);
				str_reset(name);

				int sz = (int)(names.size-1);
				str_t* s = str_new("");
					
				if(sz == 0 && load) {
					int arg_num = callFunc(l, bc);
					gen_func_name((const char*)names.items[sz], arg_num, s);
					bc_gen_str(bc, INSTR_CALL, s->cstr);	
				}
				else {
					int i;
					for(i=0; i<sz; i++) {
						bc_gen_str(bc, load ? INSTR_LOAD:INSTR_GET, (const char*)names.items[i]);	
						load = false;
					}
					int arg_num = callFunc(l, bc);
					gen_func_name((const char*)names.items[sz], arg_num, s);
					bc_gen_str(bc, INSTR_CALLO, s->cstr);	
				}
				load = false;
				array_clean(&names, NULL, NULL);
				str_free(s);
			} 
			else if (l->tk == '.') { // ------------------------------------- Record Access
				if(!lex_chkread(l, '.')) return false;
				if(name->len == 0)
					str_cpy(name, l->tk_str->cstr);
				else {
					str_append(name, ".");
					str_append(name, l->tk_str->cstr);
				}
				if(!lex_chkread(l, LEX_ID)) return false;
			} 
			else { // ------------------------------------- Array Access
				int i;
				int sz;

				str_split(name->cstr, '.', &names);
				str_reset(name);
				sz = names.size;
				for(i=0; i<sz; i++) {
					bc_gen_str(bc, load ? INSTR_LOAD:INSTR_GET, (const char*)names.items[i]);	
					load = false;
				}
				array_clean(&names, NULL, NULL);

				if(!lex_chkread(l, '[')) return false;
				if(!base(l, bc)) return false;
				if(!lex_chkread(l, ']')) return false;
				bc_gen(bc, INSTR_ARRAY_AT);
			} 
		}

		if(name->len > 0) {
			int i, sz;
			str_split(name->cstr, '.', &names);
			str_reset(name);
			sz = names.size;
			for(i=0; i<sz; i++) {
				bc_gen_str(bc, load ? INSTR_LOAD:INSTR_GET, (const char*)names.items[i]);	
				load = false;
			}
			array_clean(&names, NULL, NULL);
		}
		str_free(name);
	}
	else if (l->tk=='[') {
		// JSON-style array 
		if(!lex_chkread(l, '[')) return false;
		bc_gen(bc, INSTR_ARRAY);
		while (l->tk != ']') {
			base(l, bc);
			bc_gen(bc, INSTR_MEMBER);
			if (l->tk != ']') {
				if(!lex_chkread(l, ',')) return false;
			}
		}
		if(!lex_chkread(l, ']')) return false;
		bc_gen(bc, INSTR_ARRAY_END);
	}

	return true;
}

bool unary(lex_t* l, bytecode_t* bc) {
	opr_code_t instr = INSTR_END;
	if (l->tk == '!') {
		if(!lex_chkread(l, '!')) return false;
		instr = INSTR_NOT;
	} else if(l->tk == LEX_R_TYPEOF) {
		if(!lex_chkread(l, LEX_R_TYPEOF)) return false;
		instr = INSTR_TYPEOF;
	}

	if(!factor(l, bc))
		return false;

	if(instr != INSTR_END) {
		bc_gen(bc, instr);
	}
	return true;	
}

bool term(lex_t* l, bytecode_t* bc) {
	if(!unary(l, bc))
		return false;

	while (l->tk=='*' || l->tk=='/' || l->tk=='%') {
		LEX_TYPES op = l->tk;
		if(!lex_chkread(l, l->tk)) return false;
		if(!unary(l, bc)) return false;

		if(op == '*') {
			bc_gen(bc, INSTR_MULTI);
		}
		else if(op == '/') {
			bc_gen(bc, INSTR_DIV);
		}
		else {
			bc_gen(bc, INSTR_MOD);
		}
	}

	return true;	
}

bool expr(lex_t* l, bytecode_t* bc) {
	LEX_TYPES pre = l->tk;

	if (l->tk=='-') {
		if(!lex_chkread(l, '-')) return false;
	}
	else if(l->tk==LEX_PLUSPLUS) {
		if(!lex_chkread(l, LEX_PLUSPLUS)) return false;
	}
	else if(l->tk==LEX_MINUSMINUS) {
		if(!lex_chkread(l, LEX_MINUSMINUS)) return false;
	}

	if(!term(l, bc))
		return false;

	if (pre == '-') {
		bc_gen(bc, INSTR_NEG);
	}
	else if(pre==LEX_PLUSPLUS) {
		bc_gen(bc, INSTR_PPLUS_PRE);
	}
	else if(pre==LEX_MINUSMINUS) {
		bc_gen(bc, INSTR_MMINUS_PRE);
	}

	while (l->tk=='+' || l->tk=='-' ||
			l->tk==LEX_PLUSPLUS || l->tk==LEX_MINUSMINUS) {
		int op = l->tk;
		if(!lex_chkread(l, l->tk)) return false;
		if (op==LEX_PLUSPLUS) {
			bc_gen(bc, INSTR_PPLUS);
		}
		else if(op==LEX_MINUSMINUS) {
			bc_gen(bc, INSTR_MMINUS);
		}
		else {
			if(!term(l, bc))
				return false;
			if(op== '+') {
				bc_gen(bc, INSTR_PLUS);
			}
			else if(op=='-') {
				bc_gen(bc, INSTR_MINUS);
			}
		}
	}

	return true;	
}

bool shift(lex_t* l, bytecode_t* bc) {
	if(!expr(l, bc))
		return false;

	if (l->tk==LEX_LSHIFT || l->tk==LEX_RSHIFT || l->tk==LEX_RSHIFTUNSIGNED) {
		int op = l->tk;
		if(!lex_chkread(l, op)) return false;
		if(!base(l, bc))
			return false;

		if (op==LEX_LSHIFT) {
			bc_gen(bc, INSTR_LSHIFT);
		}
		else if (op==LEX_RSHIFT) {
			bc_gen(bc, INSTR_RSHIFT);
		}
		else {
			bc_gen(bc, INSTR_URSHIFT);
		}
	}
	return true;	
}

bool condition(lex_t *l, bytecode_t* bc) {
	if(!shift(l, bc))
		return false;

	while (l->tk==LEX_EQUAL || l->tk==LEX_NEQUAL ||
			l->tk==LEX_TYPEEQUAL || l->tk==LEX_NTYPEEQUAL ||
			l->tk==LEX_LEQUAL || l->tk==LEX_GEQUAL ||
			l->tk=='<' || l->tk=='>') {
		int op = l->tk;
		if(!lex_chkread(l, l->tk)) return false;
		if(!shift(l, bc))
			return false;

		if(op == LEX_EQUAL) {
			bc_gen(bc, INSTR_EQ);
		}
		else if(op == LEX_NEQUAL) {
			bc_gen(bc, INSTR_NEQ);
		}
		else if(op == LEX_TYPEEQUAL) {
			bc_gen(bc, INSTR_TEQ);
		}
		else if(op == LEX_NTYPEEQUAL) {
			bc_gen(bc, INSTR_NTEQ);
		}
		else if(op == LEX_LEQUAL) {
			bc_gen(bc, INSTR_LEQ);
		}
		else if(op == LEX_GEQUAL) {
			bc_gen(bc, INSTR_GEQ);
		}
		else if(op == '>') {
			bc_gen(bc, INSTR_GRT);
		}
		else if(op == '<') {
			bc_gen(bc, INSTR_LES);
		}
	}

	return true;	
}

bool logic(lex_t* l, bytecode_t* bc) {
	if(!condition(l, bc))
		return false;

	while (l->tk=='&' || l->tk=='|' || l->tk=='^' || l->tk==LEX_ANDAND || l->tk==LEX_OROR) {
		int op = l->tk;
		if(!lex_chkread(l, l->tk)) return false;
		if(!condition(l, bc))
			return false;

		if (op==LEX_ANDAND) {
			bc_gen(bc, INSTR_AAND);
		} 
		else if (op==LEX_OROR) {
			bc_gen(bc, INSTR_OOR);
		}
		else if (op=='|') {
			bc_gen(bc, INSTR_OR);
		}
		else if (op=='&') {
			bc_gen(bc, INSTR_AND);
		}
		else if (op=='^') {
			bc_gen(bc, INSTR_XOR);
		}
	}
	return true;	
}


bool ternary(lex_t *l, bytecode_t* bc) {
	if(!logic(l, bc))
		return false;
	
	if (l->tk=='?') {
		PC pc1 = bc_reserve(bc); //keep for jump
		if(!lex_chkread(l, '?')) return false;
		base(l, bc); //first choice
		PC pc2 = bc_reserve(bc); //keep for jump
		if(!lex_chkread(l, ':')) return false;
		bc_set_instr(bc, pc1, INSTR_NJMP, ILLEGAL_PC);
		base(l, bc); //second choice
		bc_set_instr(bc, pc2, INSTR_JMP, ILLEGAL_PC);
	} 
	return true;	
}

bool base(lex_t* l, bytecode_t* bc) {
	if(!ternary(l, bc))
		return false;

	if (l->tk=='=' || 
			l->tk==LEX_PLUSEQUAL ||
			l->tk==LEX_MULTIEQUAL ||
			l->tk==LEX_DIVEQUAL ||
			l->tk==LEX_MODEQUAL ||
			l->tk==LEX_MINUSEQUAL) {
		LEX_TYPES op = l->tk;
		if(!lex_chkread(l, l->tk)) return false;
		base(l, bc);
		// sort out initialiser
		if (op == '=')  {
			bc_gen(bc, INSTR_ASIGN);
		}
		else if(op == LEX_PLUSEQUAL) {
			bc_gen(bc, INSTR_PLUSEQ);
		}
		else if(op == LEX_MINUSEQUAL) {
			bc_gen(bc, INSTR_MINUSEQ);
		}
		else if(op == LEX_MULTIEQUAL) {
			bc_gen(bc, INSTR_MULTIEQ);
		}
		else if(op == LEX_DIVEQUAL) {
			bc_gen(bc, INSTR_DIVEQ);
		}
		else if(op == LEX_MODEQUAL) {
			bc_gen(bc, INSTR_MODEQ);
		}
	}
	return true;
}

bool statement(lex_t* l, bytecode_t* bc, bool pop, loop_t* loop) {
	if (l->tk=='{') {
		/* A block of code */
		if(!block(l, bc, loop, false))
			return false;
		pop = false;
	}
	else if (l->tk==LEX_ID    ||
			l->tk==LEX_INT   ||
			l->tk==LEX_FLOAT ||
			l->tk==LEX_STR   ||
			l->tk==LEX_PLUSPLUS   ||
			l->tk==LEX_MINUSMINUS ||
			l->tk=='-'    ) {
		/* Execute a simple statement that only contains basic arithmetic... */
		if(!base(l, bc))
			return false;
		if(!lex_chkread(l, ';')) return false;
	}
	else if (l->tk==LEX_R_VAR || l->tk == LEX_R_CONST || l->tk == LEX_R_LET) {
		pop = false;
		opr_code_t op;
		//bool be_const;

		if(l->tk == LEX_R_VAR) {
			if(!lex_chkread(l, LEX_R_VAR)) return false;
			//be_const = false;
			op = INSTR_VAR;
		}
		else if(l->tk == LEX_R_LET) {
			if(!lex_chkread(l, LEX_R_LET)) return false;
			//be_const = false;
			op = INSTR_LET;
		}
		else {
			if(!lex_chkread(l, LEX_R_CONST)) return false;
			//be_const = true;
			op = INSTR_CONST;
		}

		while (l->tk != ';') {
			str_t* vname = str_new(l->tk_str->cstr);
			if(!lex_chkread(l, LEX_ID)) return false;
			bc_gen_str(bc, op, vname->cstr);
			// sort out initialiser
			if (l->tk == '=') {
				if(!lex_chkread(l, '=')) return false;
				bc_gen_str(bc, INSTR_LOAD, vname->cstr);
				if(!base(l, bc)) return false;
				bc_gen(bc, INSTR_ASIGN);
				bc_gen(bc, INSTR_POP);
			}
			if (l->tk != ';')
				if(!lex_chkread(l, ',')) return false;
			str_free(vname);
		}
		if(!lex_chkread(l, ';')) return false;
	}
	else if(l->tk == LEX_R_CLASS) {
		def_class(l, bc);
	}
	else if(l->tk == LEX_R_FUNCTION) {
		if(!lex_chkread(l, LEX_R_FUNCTION)) return false;
		str_t* fname = str_new("");
		defFunc(l, bc, fname);
		bc_gen_str(bc, INSTR_MEMBERN, fname->cstr);
		str_free(fname);
		pop = false;
	}
	else if (l->tk == LEX_R_RETURN) {
		if(!lex_chkread(l, LEX_R_RETURN)) return false;
		pop = false;
		if (l->tk != ';') {
			if(!base(l, bc)) return false;
			bc_gen(bc, INSTR_RETURNV);
		}
		else {
			bc_gen(bc, INSTR_RETURN);
		}
		if(!lex_chkread(l, ';')) return false;
	} 
	else if (l->tk == LEX_R_IF) {
		if(!lex_chkread(l, LEX_R_IF)) return false;
		if(!lex_chkread(l, '(')) return false;
		if(!base(l, bc)) return false; //condition
		if(!lex_chkread(l, ')')) return false;
		PC pc = bc_reserve(bc);
		if(!statement(l, bc, true, loop)) return false;

		if (l->tk == LEX_R_ELSE) {
			if(!lex_chkread(l, LEX_R_ELSE)) return false;
			PC pc2 = bc_reserve(bc);
			bc_set_instr(bc, pc, INSTR_NJMP, ILLEGAL_PC);
			if(!statement(l, bc, true, loop)) return false;
			bc_set_instr(bc, pc2, INSTR_JMP, ILLEGAL_PC);
		}
		else {
			bc_set_instr(bc, pc, INSTR_NJMP, ILLEGAL_PC);
		}
		pop = false;
	}
	else if (l->tk == LEX_R_WHILE) {
		if(!lex_chkread(l, LEX_R_WHILE)) return false;
		PC pc = bc_gen(bc, INSTR_BLOCK);
		bc_add_instr(bc, pc, INSTR_JMP, pc+2); //jmp to loop(skip the next jump instruction).
		PC pcb = bc_reserve(bc); //jump out of loop (for break anchor);

		if(!lex_chkread(l, '(')) return false;
		PC cpc = bc->cindex; //continue anchor
		if(!base(l, bc)) return false; //condition
		if(!lex_chkread(l, ')')) return false;

		pc = bc_reserve(bc); //njmp on condition
		
		loop_t lp;
		lp.continueAnchor = cpc;
		lp.breakAnchor = pcb;
		lp.blockDepth = 0;
		
		if(!statement(l, bc, true, &lp)) return false;

		bc_add_instr(bc, cpc, INSTR_JMPB, ILLEGAL_PC); //coninue anchor;
		bc_set_instr(bc, pc, INSTR_NJMP, ILLEGAL_PC); // end anchor;
		bc_gen_short(bc, INSTR_BLOCK_END, 1);
		bc_set_instr(bc, pcb, INSTR_JMP, ILLEGAL_PC); // end anchor;
		pop = false;
	}
	else if (l->tk==LEX_R_FOR) {
		if(!lex_chkread(l, LEX_R_FOR)) return false;
		PC pc = bc_gen(bc, INSTR_BLOCK);
		bc_add_instr(bc, pc, INSTR_JMP, pc+2); //jmp to loop(skip the next jump instruction).
		PC pcb = bc_reserve(bc); //jump out of loop (for break anchor);

		if(!lex_chkread(l, '(')) return false;
		if(!statement(l, bc, true, NULL)) //init statement
			return false;

		PC cpc = bc->cindex; //condition anchor(also continue anchor as well)
		if(!base(l, bc)) //condition
			return false; 
		if(!lex_chkread(l, ';')) return false;
		pc = bc_reserve(bc); //njmp on condition for jump out of loop.
		PC lpc = bc_reserve(bc); //jmp to loop(skip iterator part).

		PC ipc = bc->cindex;  //iterator anchor;
		if(!base(l, bc)) //iterator statement
			return false; 
		if(!lex_chkread(l, ')')) return false;
		bc_gen(bc, INSTR_POP); //pop the stack.
		bc_add_instr(bc, cpc, INSTR_JMPB, ILLEGAL_PC); //jump to coninue anchor;

		loop_t lp;
		lp.continueAnchor = cpc;
		lp.breakAnchor = pcb;
		lp.blockDepth = 0;

		bc_set_instr(bc, lpc, INSTR_JMP, ILLEGAL_PC); // loop anchor;
		if(!statement(l, bc, true, &lp)) return false; //loop statement

		bc_add_instr(bc, ipc, INSTR_JMPB, ILLEGAL_PC); //jump to iterator anchor;
		bc_set_instr(bc, pc, INSTR_NJMP, ILLEGAL_PC); // end anchor;
		bc_gen_short(bc, INSTR_BLOCK_END, 1);
		bc_set_instr(bc, pcb, INSTR_JMP, ILLEGAL_PC); // end anchor;
		pop = false;
	}
	else if(l->tk == LEX_R_BREAK) {
		if(!lex_chkread(l, LEX_R_BREAK)) return false;
		if(!lex_chkread(l, ';')) return false;
		if(loop == NULL) {
			_err("Error: There is no loop for 'break' here!\n");
			return false;
		}

		bc_gen_short(bc, INSTR_BLOCK_END, loop->blockDepth);
		bc_add_instr(bc, loop->breakAnchor, INSTR_JMPB, ILLEGAL_PC); //to break anchor;
		pop = false;
	}
	else if(l->tk == LEX_R_CONTINUE) {
		if(!lex_chkread(l, LEX_R_CONTINUE)) return false;
		if(!lex_chkread(l, ';')) return false;
		if(loop == NULL) {
			_err("Error: There is no loop for 'continue' here!\n");
			return false;
		}

		bc_gen_short(bc, INSTR_BLOCK_END, loop->blockDepth);
		bc_add_instr(bc, loop->continueAnchor, INSTR_JMPB, ILLEGAL_PC); //to continue anchor;
		pop = false;
	}
	else {
			_err("Error: don't understand '");
			_err(l->tk_str->cstr);
			_err("'!\n");
			return false;
	}

	if(pop)
		bc_gen(bc, INSTR_POP);
	return true;
}

bool compile(bytecode_t *bc, const char* input) {
	lex_t lex;
	lex_init(&lex, input);

	while(lex.tk) {
		if(!statement(&lex, bc, true, NULL)) {
			lex_release(&lex);
			return false;
		}
	}
	bc_gen(bc, INSTR_END);
	lex_release(&lex);
	return true;
}

/** vm var-----------------------------*/

node_t* node_new(const char* name) {
	node_t* node = (node_t*)_malloc(sizeof(node_t));
	node->magic = 1;

	uint32_t len = strlen(name);
	node->name = (char*)_malloc(len+1);
	memcpy(node->name, name, len+1);

	node->be_const = false;
	node->var = var_new();	
	return node;
}

#ifdef MARIO_CACHE
void node_uncache(vm_t* vm, node_t* node);
#endif

void node_free(void* p, void* extra) {
	node_t* node = (node_t*)p;
	vm_t* vm = (vm_t*)extra;
	if(node == NULL)
		return;

#ifdef MARIO_CACHE
	node_uncache(vm, node);
#endif

	_free(node->name);
	var_unref(vm, node->var, true);
	_free(node);
}

inline var_t* node_replace(vm_t* vm, node_t* node, var_t* v) {
	if(node->var->type == V_INT && v->type == V_INT) {
		*(int*)(node->var->value) = *(int*)(v->value);
	}
	else if(node->var->type == V_FLOAT && v->type == V_FLOAT) {
		*(float*)(node->var->value) = *(float*)(v->value);
	}
	else {
		var_t* old = node->var;
		node->var = var_ref(v);
		var_unref(vm, old, true);
	}
	return node->var;
}

inline void var_remove_all(vm_t* vm, var_t* var) {
	/*free children*/
	array_clean(&var->children, node_free, vm);
}

node_t* var_add(vm_t* vm, var_t* var, const char* name, var_t* add) {
	node_t* node = NULL;

	if(name[0] != 0) 
		node = var_find(var, name);

	if(node == NULL) {
		node = node_new(name);
		var_ref(node->var);
		array_add(&var->children, node);
	}

	if(add != NULL)
		node_replace(vm, node, add);

	return node;
}

inline node_t* var_find(var_t* var, const char*name) {
	uint32_t i;

	for(i=0; i<var->children.size; i++) {
		node_t* node = (node_t*)var->children.items[i];
		if(node != NULL) {
			if(strcmp(node->name, name) == 0) {
				return node;
			}
		}
	}
	return NULL;
}

inline var_t* var_find_var(var_t* var, const char*name) {
	node_t* node = var_find(var, name);
	if(node == NULL)
		return NULL;
	return node->var;
}

inline node_t* var_find_create(vm_t* vm, var_t* var, const char*name) {
	node_t* n = var_find(var, name);
	if(n != NULL)
		return n;
	n = var_add(vm, var, name, NULL);
	return n;
}

node_t* var_get(vm_t* vm, var_t* var, int32_t index) {
	int32_t i;
	for(i=var->children.size; i<=index; i++) {
		var_add(vm, var, "", NULL);
	}

	node_t* node = (node_t*)array_get(&var->children, index);
	return node;
}

void func_free(void* p, void* extra);

inline void var_free(void* p, void* extra) {
	var_t* var = (var_t*)p;
	vm_t* vm = (vm_t*)extra;
	if(var == NULL || var->refs > 0)
		return;

	/*free children*/
	if(var->children.size > 0)
		var_remove_all(vm, var);	

	/*free value*/
	if(var->value != NULL) {
		if(var->free_func != NULL) 
			var->free_func(var->value, vm);
		else
			_free(var->value);
	}

	if(var->on_destroy != NULL) {
		var->on_destroy(var, vm);
	}
	_free(var);
}

inline var_t* var_ref(var_t* var) {
	//	if(var != NULL)
	++var->refs;
	return var;
}

inline void var_unref(vm_t* vm, var_t* var, bool del) {
	//	if(var != NULL) {
	if(var->refs > 0)
		--var->refs;

	if(var->refs == 0 && del)
		var_free(var, vm);
	//	}
}

const char* get_typeof(var_t* var) {
	switch(var->type) {
		case V_UNDEF:
			return "undefined";
		case V_INT:
		case V_FLOAT:
			return "number";
		case V_BOOL: 
			return "boolean";
		case V_STRING: 
			return "string";
		case V_OBJECT: 
			return var->is_func ? "function": "object";
	}
	return "undefined";
}

inline var_t* var_new() {
	var_t* var = (var_t*)_malloc(sizeof(var_t));
	var->magic = 0;
	var->refs = 0;
	var->type = V_UNDEF;
	var->size = 0;

	var->value = NULL;
	var->free_func = NULL;
	var->on_destroy = NULL;
	array_init(&var->children);
	return var;
}

inline var_t* var_new_block() {
	var_t* var = var_new();
	var->type = V_BLOCK;
	return var;
}

inline var_t* var_new_array() {
	var_t* var = var_new_obj(NULL, NULL);
	var->is_array = 1;
	return var;
}

inline var_t* var_new_int(int i) {
	var_t* var = var_new();
	var->type = V_INT;
	var->value = _malloc(sizeof(int));
	*((int*)var->value) = i;
	return var;
}

inline var_t* var_new_bool(bool b) {
	var_t* var = var_new();
	var->type = V_BOOL;
	var->value = _malloc(sizeof(int));
	*((int*)var->value) = b;
	return var;
}

inline var_t* var_new_obj(void*p, free_func_t fr) {
	var_t* var = var_new();
	var->type = V_OBJECT;
	var->value = p;
	var->free_func = fr;
	return var;
}

inline var_t* var_new_float(float i) {
	var_t* var = var_new();
	var->type = V_FLOAT;
	var->value = _malloc(sizeof(float));
	*((float*)var->value) = i;
	return var;
}

inline var_t* var_new_str(const char* s) {
	var_t* var = var_new();
	var->type = V_STRING;
	var->size = strlen(s);
	var->value = _malloc(var->size + 1);
	memcpy(var->value, s, var->size + 1);
	return var;
}

inline const char* var_get_str(var_t* var) {
	if(var == NULL || var->value == NULL)
		return "";
	
	return (const char*)var->value;
}

inline bool var_get_bool(var_t* var) {
	if(var == NULL || var->value == NULL)
		return false;
	int i = (int)(*(int*)var->value);
	return i==0 ? false:true;
}

inline int var_get_int(var_t* var) {
	if(var == NULL || var->value == NULL)
		return 0;
	if(var->type == V_FLOAT)	
		return (int)(*(float*)var->value);
	return *(int*)var->value;
}

inline float var_get_float(var_t* var) {
	if(var == NULL || var->value == NULL)
		return 0.0;
	
	if(var->type == V_INT)	
		return (float)(*(int*)var->value);
	return *(float*)var->value;
}

inline func_t* var_get_func(var_t* var) {
	if(var == NULL || var->value == NULL)
		return NULL;
	
	return (func_t*)var->value;
}

void get_js_str(const char* str, str_t* ret) {
	str_reset(ret);
	str_add(ret, '"');
	/*
	while(*str != 0) {
		switch (*str) {
			case '\\': str_append(ret, "\\\\"); break;
			case '\n': str_append(ret, "\\n"); break;
			case '\r': str_append(ret, "\\r"); break;
			case '\a': str_append(ret, "\\a"); break;
			case '"':  str_append(ret, "\\\""); break;
			default: str_add(ret, *str);
		}
		str++;
	}
	*/
	str_append(ret, str);
	str_add(ret, '"');
}

void var_to_str(vm_t* vm, var_t* var, str_t* ret) {
	str_reset(ret);
	if(var == NULL) {
		str_cpy(ret, "undefined");
		return;
	}

	char s[FROM_STR_MAX];
	switch(var->type) {
	case V_INT:
		str_cpy(ret, str_from_int(var_get_int(var), s));
		break;
	case V_FLOAT:
		str_cpy(ret, str_from_float(var_get_float(var), s));
		break;
	case V_STRING:
		str_cpy(ret, var_get_str(var));
		break;
	case V_OBJECT:
		var_to_json_str(vm, var, ret, 0);
		break;
	case V_BOOL:
		str_cpy(ret, var_get_int(var) == 1 ? "true":"false");
		break;
	default:
		str_cpy(ret, "undefined");
		break;
	}
}

void get_parsable_str(vm_t* vm, var_t* var, str_t* ret) {
	str_reset(ret);

	str_t* s = str_new("");
	var_to_str(vm, var, s);
	if(var->type == V_STRING)
		get_js_str(s->cstr, ret);
	else
		str_cpy(ret, s->cstr);

	str_free(s);
}

void append_json_spaces(str_t* ret, int level) {
	int spaces;
	for (spaces = 0; spaces<=level; ++spaces) {
		str_add(ret, ' '), str_add(ret, ' ');
	}
}

static bool _done_arr_inited = false;
void var_to_json_str(vm_t* vm, var_t* var, str_t* ret, int level) {
	str_reset(ret);

	uint32_t i;

	//check if done to avoid dead recursion
	static m_array_t done;
	if(level == 0) {
		if(!_done_arr_inited) {		
			array_init(&done);
			_done_arr_inited = true;
		}
		array_remove_all(&done);
	}
	uint32_t sz = done.size;
	for(i=0; i<sz; ++i) {
		if(done.items[i] == var) { //already done before.
			str_cpy(ret, "{}");
			if(level == 0)
				array_remove_all(&done);
			return;
		}
	}
	array_add(&done, var);

	if (var->is_array) {
		str_add(ret, '[');
		int len = (int)var->children.size;
		if (len>100) len=100; // we don't want to get stuck here!

		int i;
		for (i=0;i<len;i++) {
			node_t* n = var_get(vm, var, i);

			str_t* s = str_new("");
			var_to_json_str(vm, n->var, s, level);
			str_append(ret, s->cstr);
			str_free(s);

			if (i<len-1) 
				str_append(ret, ", ");
		}
		str_add(ret, ']');
	}
	else if (var->is_func) {
		str_append(ret, "function (");
		// get list of parameters
		int sz = 0;
		if(var->value != NULL) {
			func_t* func = var_get_func(var);
			sz = func->args.size;
			int i=0;
			for(i=0; i<sz; ++i) {
				str_append(ret, (const char*)func->args.items[i]);
				if ((i+1) < sz) {
					str_append(ret, ", ");
				}
			}
		}
		// add function body
		str_append(ret, ") {}");
		return;
	}
	else if (var->type == V_OBJECT) {
		// children - handle with bracketed list
		int sz = (int)var->children.size;
		if(sz > 0)
			str_append(ret, "{\n");
		else
			str_append(ret, "{");

		int i;
		for(i=0; i<sz; ++i) {
			node_t* n = var_get(vm, var, i);
			append_json_spaces(ret, level);
			str_add(ret, '"');
			str_append(ret, n->name);
			str_add(ret, '"');
			str_append(ret, ": ");

			str_t* s = str_new("");
			var_to_json_str(vm, n->var, s, level+1);
			str_append(ret, s->cstr);
			str_free(s);

			if ((i+1) < sz) {
				str_append(ret, ",\n");
			}
		}
		if(sz > 0) {
			str_add(ret, '\n');
		}
	
		append_json_spaces(ret, level - 1);	
		str_add(ret, '}');
	} 
	else {
		// no children or a function... just write value directly
		str_t* s = str_new("");
		get_parsable_str(vm, var, s);
		str_append(ret, s->cstr);
		str_free(s);
	}

	if(level == 0) {
		array_remove_all(&done);
	}
}

/** var cache for const value --------------*/

#ifdef MARIO_CACHE

void var_cache_init(vm_t* vm) {
	uint32_t i;
	for(i=0; i<vm->var_cache_used; ++i) {
		vm->var_cache[i] = NULL;
	}
	vm->var_cache_used = 0;	
}

void var_cache_free(vm_t* vm) {
	uint32_t i;
	for(i=0; i<vm->var_cache_used; ++i) {
		var_t* v = vm->var_cache[i];
		var_unref(vm, v, true);
		vm->var_cache[i] = NULL;
	}
	vm->var_cache_used = 0;	
}

int32_t var_cache(vm_t* vm, var_t* v) {
	if(vm->var_cache_used >= VAR_CACHE_MAX)
		return -1;
	vm->var_cache[vm->var_cache_used] = var_ref(v);
	vm->var_cache_used++;
	return vm->var_cache_used-1;
}

bool try_cache(vm_t* vm, PC* ins, var_t* v) {
	if((*ins) & INSTR_NEED_IMPROVE) {
		int index = var_cache(vm, v); 
		if(index >= 0) 
			*ins = (INSTR_CACHE << 16 | index);
		return true;
	}

	*ins = (*ins) | INSTR_NEED_IMPROVE;
	return false;
}

void node_cache_init(vm_t* vm) {
	int i; 
	for(i=0; i<NODE_CACHE_MAX; ++i) {
		vm->node_cache[i].node = NULL;
		vm->node_cache[i].sc_var = NULL;
	}
}

int node_cache(vm_t* vm, var_t* sc_var, node_t* node, int name_id) {
	int i; 

	for(i=0; i<NODE_CACHE_MAX; ++i) {
		if(vm->node_cache[i].node == node &&
				vm->node_cache[i].sc_var == sc_var)
			return i;
	}

	for(i=0; i<NODE_CACHE_MAX; ++i) {
		if(vm->node_cache[i].node == NULL) {
			vm->node_cache[i].node = node;
			vm->node_cache[i].sc_var = sc_var;
			vm->node_cache[i].name_id = name_id;
			return i;
		}
	}
	return -1;
}

void node_uncache(vm_t* vm, node_t* node) {
	int i; 
	for(i=0; i<NODE_CACHE_MAX; ++i) {
		if(vm->node_cache[i].node == node) {
			vm->node_cache[i].node = NULL;
			vm->node_cache[i].sc_var = NULL;
		}
	}
}

#endif

/** JSON Parser-----------------------------*/

var_t* json_parse_factor(vm_t* vm, lex_t *l) {
	if (l->tk==LEX_R_TRUE) {
		lex_chkread(l, LEX_R_TRUE);
		return var_new_int(1);
	}
	else if (l->tk==LEX_R_FALSE) {
		lex_chkread(l, LEX_R_FALSE);
		return var_new_int(0);
	}
	else if (l->tk==LEX_R_NULL) {
		lex_chkread(l, LEX_R_NULL);
		return var_new();
	}
	else if (l->tk==LEX_R_UNDEFINED) {
		lex_chkread(l, LEX_R_UNDEFINED);
		return var_new();
	}
	else if (l->tk==LEX_INT) {
		int i = atoi(l->tk_str->cstr);
		lex_chkread(l, LEX_INT);
		return var_new_int(i);
	}
	else if (l->tk==LEX_FLOAT) {
		float f = atof(l->tk_str->cstr);
		lex_chkread(l, LEX_FLOAT);
		return var_new_float(f);
	}
	else if (l->tk==LEX_STR) {
		str_t* s = str_new(l->tk_str->cstr);
		lex_chkread(l, LEX_STR);
		var_t* ret = var_new_str(s->cstr);
		str_free(s);
		return ret;
	}
	else if(l->tk==LEX_R_FUNCTION) {
		lex_chkread(l, LEX_R_FUNCTION);
		//TODO
		_err("Error: Can not parse json function item!\n");
		return var_new();
	}
	else if (l->tk=='[') {
		/* JSON-style array */
		var_t* arr = var_new_array();
		lex_chkread(l, '[');
		while (l->tk != ']') {
			var_t* v = json_parse_factor(vm, l);
			var_add(vm, arr, "", v);
			if (l->tk != ']') 
				lex_chkread(l, ',');
		}
		lex_chkread(l, ']');
		return arr;
	}
	else if (l->tk=='{') {
		lex_chkread(l, '{');
		var_t* obj = var_new_obj(NULL, NULL);
		while(l->tk != '}') {
			str_t* id = str_new(l->tk_str->cstr);
			if(l->tk == LEX_STR)
				lex_chkread(l, LEX_STR);
			else
				lex_chkread(l, LEX_ID);

			lex_chkread(l, ':');
			var_t* v = json_parse_factor(vm, l);
			var_add(vm, obj, id->cstr, v);
			str_free(id);
			if(l->tk != '}')
				lex_chkread(l, ',');
		}
		lex_chkread(l, '}');
		return obj;
	}
	return var_new();
}

var_t* json_parse(vm_t* vm, const char* str) {
	lex_t lex;
	lex_init(&lex, str);
	var_t* ret = json_parse_factor(vm, &lex);
	lex_release(&lex);
	return ret;
}

/** Interpreter-----------------------------*/

inline void vm_push(vm_t* vm, var_t* var) {  
	var_ref(var);
	if(vm->stack_top < VM_STACK_MAX) {
		vm->stack[vm->stack_top++] = var; 
	} 
}

inline void vm_push_node(vm_t* vm, node_t* node) {
	var_ref(node->var); 
	if(vm->stack_top < VM_STACK_MAX)
		vm->stack[vm->stack_top++] = node;
}

static inline var_t* vm_pop2(vm_t* vm) {
	void *p = NULL;
	vm->stack_top--;
	p = vm->stack[vm->stack_top];
	int8_t magic = *(int8_t*)p;
	//var
	if(magic == 0) {
		return(var_t*)p;
	}

	//node
	node_t* node = (node_t*)p;
	if(node != NULL)
		return node->var;

	return NULL;
}

/*#define vm_pop2(vm) ({ \
	(vm)->stack_top--; \
	void* p = (vm)->stack[(vm)->stack_top]; \
	int8_t magic = *(int8_t*)p; \
	var_t* ret = NULL; \
	if(magic == 0) { \
		ret = (var_t*)p; \
	} \
	else { \
	node_t* node = (node_t*)p; \
	if(node != NULL) \
		ret = node->var; \
	else \
		ret = NULL; \
	} \
	ret; \
})
*/

static inline void vm_pop(vm_t* vm) {
	vm->stack_top--;
	void *p = vm->stack[vm->stack_top];
	int8_t magic = *(int8_t*)p;
	var_t* v;
	if(magic == 0) { //var
		v = (var_t*)p;
	}
	else { //node
		node_t* node = (node_t*)p;
		v = node->var;
		str_t* s = str_new("");
		var_to_str(vm, v, s);
		str_add(s, '\n');
		_out_func(s->cstr);
		str_free(s);
	}

	var_unref(vm, v, true);
}

static inline node_t* vm_pop2node(vm_t* vm) {
	void *p = NULL;
	/*if(vm->stack_top <= 0) 
		return NULL;
	*/

	vm->stack_top--;
	p = vm->stack[vm->stack_top];
	int8_t magic = *(int8_t*)p;
	if(magic != 1) {//not node!
		return NULL;
	}

	return (node_t*)p;
}

var_t* vm_stack_pick(vm_t* vm, int depth) {
	int index = vm->stack_top - depth;
	if(index < 0)
		return NULL;

	void* p = vm->stack[index];
	var_t* ret = NULL;
	if(p == NULL)
		return ret;

	int8_t magic = *(int8_t*)p;
	if(magic == 1) {//node
		node_t* node = (node_t*)p;
		ret = node->var;
	}
	else {
		ret = (var_t*)p;
	}

	vm->stack_top--;
	int i;
	for(i=index; i<vm->stack_top; ++i) {
		vm->stack[i] = vm->stack[i+1];
	}
	return ret;
}

//scope of vm runing
typedef struct st_scope {
	struct st_scope* prev;
	var_t* var;
	PC pc; //stack pc
	//continue and break anchor for loop(while/for)
} scope_t;

scope_t* scope_new(var_t* var, PC pc) {
	scope_t* sc = (scope_t*)_malloc(sizeof(scope_t));
	sc->prev = NULL;

	if(var != NULL)
		sc->var = var_ref(var);
	sc->pc = pc;
	return sc;
}

void scope_free(void* p, void* extra) {
	scope_t* sc = (scope_t*)p;
	vm_t* vm = (vm_t*)extra;
	if(sc == NULL)
		return;
	if(sc->var != NULL)
		var_unref(vm, sc->var, true);
	_free(sc);
}

#define vm_get_scope(vm) (scope_t*)array_tail(&(vm)->scopes)

#define vm_get_scope_var(vm, skipBlock) ({ \
	scope_t* sc = (scope_t*)array_tail(&(vm)->scopes); \
	if((skipBlock) && sc != NULL && sc->var->type == V_BLOCK) \
		sc = sc->prev; \
	var_t* ret; \
	if(sc == NULL) \
		ret = (vm)->root; \
	else \
		ret = sc->var; \
	ret; \
})

void vm_push_scope(vm_t* vm, scope_t* sc) {
	scope_t* prev = NULL;
	if(vm->scopes.size > 0)
		prev = (scope_t*)array_tail(&vm->scopes);
	array_add(&vm->scopes, sc);	
	sc->prev = prev;
}

PC vm_pop_scope(vm_t* vm) {
	if(vm->scopes.size <= 0)
		return 0;

	PC pc = 0;
	scope_t* sc = vm_get_scope(vm);
	if(sc == NULL)
		return 0;

	if(sc->pc != 0xFFFFFFFF)
		pc = sc->pc;
	array_del(&vm->scopes, vm->scopes.size-1, scope_free, vm);
	return pc;
}

void vm_stack_free(vm_t* vm, void* p) {
	int8_t magic = *(int8_t*)p;
	if(magic == 1) {//node
		node_t* node = (node_t*)p;
		node_free(node, vm);
	}
	else {
		var_t* var = (var_t*)p;
		var_free(var, vm);
	}
}

node_t* vm_find(vm_t* vm, const char* name) {
	var_t* var = vm_get_scope_var(vm, true);
	if(var == NULL)
		return NULL;
	return var_find(var, name);	
}

node_t* vm_find_in_class(var_t* var, const char* name) {
	node_t* n = var_find(var, PROTOTYPE);

	while(n != NULL && n->var != NULL && n->var->type == V_OBJECT) {
		node_t* ret = NULL;
		ret = var_find(n->var, name);
		if(ret != NULL)
			return ret;
		n = var_find(n->var, SUPER);
	}
	return NULL;
}

node_t* find_member(var_t* obj, const char* name) {
	node_t* node = var_find(obj, name);
	if(node == NULL) { 
		node = vm_find_in_class(obj, name);
	}
	return node;
}

static inline var_t* vm_this_in_scopes(vm_t* vm) {
	scope_t* sc = vm_get_scope(vm);
	while(sc != NULL) {
		if(sc->var != NULL && 
				sc->var->type == V_OBJECT) {
			return sc->var;
		}
		sc = sc->prev;
	}
	return vm->root;
}

static inline node_t* vm_find_in_scopes(vm_t* vm, const char* name) {
	node_t* ret = NULL;
	scope_t* sc = vm_get_scope(vm);
	
	/*
	if(sc != NULL && sc->var != NULL) {
		ret = var_find(sc->var, name);
		if(ret != NULL)
			return ret;

		node_t* n = var_find(sc->var, THIS);
		if(n != NULL)  {
			ret = find_member(n->var, name);
			if(ret != NULL)
				return ret;
		}
		sc = sc->prev;
	}
	*/

	while(sc != NULL) {
		if(sc->var != NULL) {
			ret = var_find(sc->var, name);
			if(ret != NULL)
				return ret;
		}
		sc = sc->prev;
	}

	return var_find(vm->root, name);
}

static inline node_t* vm_load_node(vm_t* vm, const char* name, bool create) {
	var_t* var = vm_get_scope_var(vm, true);

	node_t* n;
	if(var != NULL) {
		n = find_member(var, name);
	}
	else {
		n = vm_find_in_scopes(vm, name);
	}

	if(n == NULL)
		n =  vm_find_in_scopes(vm, name);	

	if(n != NULL)
		return n;
	/*
	_err("Warning: '");	
	_err(name);
	_err("' undefined!\n");	
	*/
	if(!create)
		return NULL;

	if(var == NULL)
		return NULL;

	n =var_add(vm, var, name, NULL);	
	return n;
}

//for function.

var_t* add_prototype(vm_t* vm, var_t* var, var_t* father) {
	var_t* v = var_new_obj(NULL, NULL);
	node_t* protoN = var_add(vm, var, PROTOTYPE, v); //add prototype object

	if(father != NULL) {
		v = get_obj(father, PROTOTYPE); //get father's prototype.
		if(v != NULL) {
			var_add(vm, protoN->var, SUPER, v);
		}
	}

	return protoN->var;
}

var_t* get_prototype(var_t* var) {
	return get_obj(var, PROTOTYPE);
}

func_t* func_new() {
	func_t* func = (func_t*)_malloc(sizeof(func_t));
	if(func == NULL)
		return NULL;
	
	func->native = NULL;
	func->regular = true;
	func->pc = 0;
	func->data = NULL;
	func->owner = NULL;
	array_init(&func->args);
	return func;
}

void func_free(void* p, void* extra) {
	func_t* func = (func_t*)p;
	array_clean(&func->args, NULL, NULL);
	_free(p);
}

var_t* var_new_func(vm_t* vm, func_t* func) {
	var_t* var = var_new_obj(NULL, NULL);
	var->is_func = 1;
	var->free_func = func_free;
	var->value = func;

	add_prototype(vm, var, vm->var_Object);
	//var_t* protoV = add_prototype(var, vm->var_Object);
	//var_add(protoV, CONSTRUCTOR, var);
	return var;
}

var_t* var_new_func_from(vm_t* vm, var_t* func_var) {
	var_t* var = var_new_obj(NULL, NULL);
	var->free_func = _free_none;
	var->value = var_get_func(func_var);

	var_t* protoV = get_prototype(func_var);
	var_add(vm, var, PROTOTYPE, protoV);

	//var_t* protoV = add_prototype(var, func_var);
	//var_add(protoV, CONSTRUCTOR, var);
	return var;
}

var_t* find_func(vm_t* vm, var_t* obj, const char* fname) {
	//try full name with arg_num
	node_t* node;
	if(obj != NULL) {
		node = find_member(obj, fname);
	}
	else {
		node = vm_find_in_scopes(vm, fname);
	}

	if(node != NULL && node->var != NULL && node->var->type == V_OBJECT) {
		return node->var;
	}
	return NULL;
}

bool func_call(vm_t* vm, var_t* obj, var_t* func_var, int arg_num) {
	var_t *env;
	func_t* func = var_get_func(func_var);
	if(obj == NULL || func->is_static) {
		obj = vm_this_in_scopes(vm);
		if(obj == vm->root) 
			obj = var_new_func_from(vm, func_var);
	}
	env = obj;

	int32_t i;
	for(i=arg_num; i>func->args.size; i--) {
		vm_pop(vm);
	}

	for(i=(int32_t)func->args.size-1; i>=0; i--) {
		const char* arg_name = (const char*)array_get(&func->args, i);
		var_t* v = NULL;
		if(i >= arg_num) {
			v = var_new();
			var_ref(v);
		}
		else {
			v = vm_pop2(vm);	
		}	
		if(v != NULL) {
			var_add(vm, env, arg_name, v);
			var_unref(vm, v, true);
		}
	}

	if(func->owner != NULL) {
		node_t* superN = var_find(func->owner, SUPER);
		if(superN != NULL)
			var_add(vm, env, SUPER, superN->var);
	}

	//native function
	if(func->native != NULL) {
		var_ref(env);
		var_t* ret = func->native(vm, env, func->data);
		if(ret == NULL)
			ret = var_new();
		vm_push(vm, ret);
		var_unref(vm, env, true);
		return true;
	}

	scope_t* sc = scope_new(env, vm->pc);
	vm_push_scope(vm, sc);

	//js function
	vm->pc = func->pc;
	vm_run(vm);
	return true;
}

var_t* func_def(vm_t* vm, bool regular, bool is_static) {
	func_t* func = func_new();
	func->regular = regular;
	func->is_static = is_static;
	while(true) {
		PC ins = vm->bc.code_buf[vm->pc++];
		opr_code_t instr = ins >> 16;
		opr_code_t offset = ins & 0x0000FFFF;
		if(instr == INSTR_JMP) {
			func->pc = vm->pc;
			vm->pc = vm->pc + offset - 1;
			break;
		}

		const char* s = bc_getstr(&vm->bc, offset);
		if(s == NULL)
			break;
		array_add_buf(&func->args, (void*)s, strlen(s) + 1);
	}

	var_t* ret = var_new_func(vm, func);
	return ret;
}


static inline void math_op(vm_t* vm, opr_code_t op, var_t* v1, var_t* v2) {
	/*if(v1->value == NULL || v2->value == NULL) {
		vm_push(vm, var_new());
		return;
	}	
	*/

	//do int
	if(v1->type == V_INT && v2->type == V_INT) {
		int i1, i2, ret = 0;
		i1 = *(int*)v1->value;
		i2 = *(int*)v2->value;

		switch(op) {
			case INSTR_PLUS: 
			case INSTR_PLUSEQ: 
				ret = (i1 + i2);
				break; 
			case INSTR_MINUS: 
			case INSTR_MINUSEQ: 
				ret = (i1 - i2);
				break; 
			case INSTR_DIV: 
			case INSTR_DIVEQ: 
				ret = (i1 / i2);
				break; 
			case INSTR_MULTI: 
			case INSTR_MULTIEQ: 
				ret = (i1 * i2);
				break; 
			case INSTR_MOD: 
			case INSTR_MODEQ: 
				ret = i1 % i2;
				break; 
			case INSTR_RSHIFT: 
				ret = i1 >> i2;
				break; 
			case INSTR_LSHIFT: 
				ret = i1 << i2;
				break; 
			case INSTR_AND: 
				ret = i1 & i2;
				break; 
			case INSTR_OR: 
				ret = i1 | i2;
				break; 
		}

		var_t* v;
		if(op == INSTR_PLUSEQ || 
				op == INSTR_MINUSEQ ||
				op == INSTR_DIVEQ ||
				op == INSTR_MULTIEQ ||
				op == INSTR_MODEQ)  {
			v = v1;
			*(int*)v->value = ret;
		}
		else {
			v = var_new_int(ret);
		}
		vm_push(vm, v);
		return;
	}

	//do float
	if(v1->type == V_FLOAT || v2->type == V_FLOAT) {
		float f1, f2, ret = 0.0;

		if(v1->type == V_FLOAT)
			f1 = *(float*)v1->value;
		else //INT
			f1 = (float) *(int*)v1->value;

		if(v2->type == V_FLOAT)
			f2 = *(float*)v2->value;
		else //INT
			f2 = (float) *(int*)v2->value;

		switch(op) {
			case INSTR_PLUS: 
			case INSTR_PLUSEQ: 
				ret = (f1 + f2);
				break; 
			case INSTR_MINUS: 
			case INSTR_MINUSEQ: 
				ret = (f1 - f2);
				break; 
			case INSTR_DIV: 
			case INSTR_DIVEQ: 
				ret = (f1 / f2);
				break; 
			case INSTR_MULTI: 
			case INSTR_MULTIEQ: 
				ret = (f1 * f2);
				break; 
		}

		var_t* v;
		if(op == INSTR_PLUSEQ || 
				op == INSTR_MINUSEQ ||
				op == INSTR_DIVEQ ||
				op == INSTR_MULTIEQ ||
				op == INSTR_MODEQ)  {
			v = v1;
			*(float*)v->value = ret;
		}
		else {
			v = var_new_float(ret);
		}
		vm_push(vm, v);
		return;
	}

	//do string + 
	if(op == INSTR_PLUS || op == INSTR_PLUSEQ) {
		str_t* s = str_new((const char*)v1->value);
		char sfrom[FROM_STR_MAX];
		switch(v2->type) {
			case V_STRING: 
				str_append(s, var_get_str(v2));
				break;
			case V_INT: 
				str_append(s, str_from_int(var_get_int(v2), sfrom));
				break;
			case V_FLOAT: 
				str_append(s, str_from_float(var_get_float(v2), sfrom));
				break;
			case V_BOOL: 
				str_append(s, str_from_bool(var_get_bool(v2)));
				break;
			/*
			case BC_var::ARRAY: 
			case BC_var::OBJECT: 
				ostr << s << v2->getJSON();
				break;
			*/
		}

		var_t* v;
		if(op == INSTR_PLUSEQ || op == INSTR_MINUSEQ) {
			v = v1;
			v->value = _realloc(v->value, s->len+1);
			memcpy(v->value, s->cstr, s->len+1);
		}
		else {
			v = var_new_str(s->cstr);
		}
		str_free(s);
		vm_push(vm, v);
	}
}

static inline void compare(vm_t* vm, opr_code_t op, var_t* v1, var_t* v2) {
	//do int
	if(v1->type == V_INT && v2->type == V_INT) {
		register int i1, i2;
		i1 = *(int*)v1->value;
		i2 = *(int*)v2->value;

		bool i = false;
		switch(op) {
			case INSTR_EQ: 
			case INSTR_TEQ:
				i = (i1 == i2);
				break; 
			case INSTR_NEQ: 
			case INSTR_NTEQ:
				i = (i1 != i2);
				break; 
			case INSTR_LES: 
				i = (i1 < i2);
				break; 
			case INSTR_GRT: 
				i = (i1 > i2);
				break; 
			case INSTR_LEQ: 
				i = (i1 <= i2);
				break; 
			case INSTR_GEQ: 
				i = (i1 >= i2);
				break; 
		}
		if(i)
			vm_push(vm, vm->var_true);
		else
			vm_push(vm, vm->var_false);
		return;
	}

	
	register float f1, f2;
	if(v1->value == NULL)
		f1 = 0.0;
	else if(v1->type == V_FLOAT)
		f1 = *(float*)v1->value;
	else //INT
		f1 = (float) *(int*)v1->value;

	if(v2->value == NULL)
		f2 = 0.0;
	else if(v2->type == V_FLOAT)
		f2 = *(float*)v2->value;
	else //INT
		f2 = (float) *(int*)v2->value;

	bool i = false;
	if(v1->type == v2->type || 
			((v1->type == V_INT || v1->type == V_FLOAT) &&
			(v2->type == V_INT || v2->type == V_FLOAT))) {
		if(v1->type == V_STRING) {
			switch(op) {
				case INSTR_EQ: 
				case INSTR_TEQ:
					i = (strcmp((const char*)v1->value, (const char*)v2->value) == 0);
					break; 
				case INSTR_NEQ: 
				case INSTR_NTEQ:
					i = (strcmp((const char*)v1->value, (const char*)v2->value) != 0);
					break;
			}
		}
		else if(v1->type == V_INT || v1->type == V_FLOAT) {
			switch(op) {
				case INSTR_EQ: 
				case INSTR_TEQ:
					i = (f1 == f2);
					break; 
				case INSTR_NEQ: 
				case INSTR_NTEQ:
					i = (f1 != f2);
					break; 
				case INSTR_LES: 
					i = (f1 < f2);
					break; 
				case INSTR_GRT: 
					i = (f1 > f2);
					break; 
				case INSTR_LEQ: 
					i = (f1 <= f2);
					break; 
				case INSTR_GEQ: 
					i = (f1 >= f2);
					break; 
			}
		}
	}
	else if(op == INSTR_NEQ || op == INSTR_NTEQ) {
		i = true;
	}

	if(i)
		vm_push(vm, vm->var_true);
	else
		vm_push(vm, vm->var_false);
}

void do_get(vm_t* vm, var_t* v, const char* name) {
	if(v->type == V_STRING && strcmp(name, "length") == 0) {
		int len = strlen(var_get_str(v));
		vm_push(vm, var_new_int(len));
		return;
	}
	else if(v->is_array && strcmp(name, "length") == 0) {
		int len = v->children.size;
		vm_push(vm, var_new_int(len));
		return;
	}	

	node_t* n = find_member(v, name);
	if(n != NULL) {
		/*if(n->var->type == V_FUNC) {
			func_t* func = var_get_func(n->var);
			if(!func->regular) { //class get/set function.
				func_call(vm, v, funC);
				return;
			}
		}
		*/
	}
	else {
		if(v->type == V_UNDEF)
			v->type = V_OBJECT;

		if(v->type == V_OBJECT)
			n = var_add(vm, v, name, NULL);
		else {
			_err("Can not get member '");
			_err(name);
			_err("'!\n");
			n = node_new(name);
			vm_push(vm, var_new());
			return;
		}
	}

	vm_push_node(vm, n);
}

void doExtends(vm_t* vm, var_t* clsProto, const char* super_name) {
	node_t* n = vm_find_in_scopes(vm, super_name);
	if(n == NULL) {
		_err("Super Class '");
		_err(super_name);
		_err("' not found!\n");
		return;
	}

	var_t* protoV = get_prototype(n->var);
	if(protoV != NULL)
		var_add(vm, clsProto, SUPER, protoV);
}

/** create object by classname or function */
var_t* new_obj(vm_t* vm, const char* name, int arg_num) {
	var_t* obj = NULL;
	node_t* n = vm_load_node(vm, name, false); //load class;

	if(n == NULL || n->var->type != V_OBJECT) {
		_err("Error: There is no class: '");
		_err(name);
		_err("'!\n");
		return var_new();
	}

	var_t* protoV = get_prototype(n->var);
	obj = var_new_obj(NULL, NULL);
	var_add(vm, obj, PROTOTYPE, protoV);

	var_t* constructor = NULL;
	if(n->var->is_func) { // new object built by function call
		constructor = n->var;
	}
	else {
		constructor = var_find_var(protoV, CONSTRUCTOR);
	}

	if(constructor != NULL) {
		func_call(vm, obj, constructor, arg_num);
		obj = vm_pop2(vm);
		var_unref(vm, obj, false);
	}
	return obj;
}

/** create object and try constructor */
void do_new(vm_t* vm, const char* full) {
	str_t* name = str_new("");
	int arg_num = parse_func_name(full, name);

	var_t* obj = new_obj(vm, name->cstr, arg_num);
	str_free(name);

	vm_push(vm, obj);
}

var_t* call_js_func(vm_t* vm, var_t* obj, var_t* func, var_t* args) {
	//push args to stack.
	int arg_num = 0;
	if(args != NULL) {
		arg_num = args->children.size;
		int i;
		for(i=0; i<arg_num; i++) {
			var_t* v = ((node_t*)args->children.items[i])->var;
			vm_push(vm, v);
		}
	}

	func_call(vm, obj, func, arg_num);
	return vm_pop2(vm);
}

var_t* call_js_func_by_name(vm_t* vm, var_t* obj, const char* func_name, var_t* args) {
	node_t* func = find_member(obj, func_name);
	if(func == NULL || func->var->is_func == 0) {
		_err("Interrupt function '");
		_err(func_name);
		_err("' not defined!\n");
		return NULL;
	}
	return call_js_func(vm, obj, func->var, args);
}

#ifdef MARIO_THREAD
/**
interrupter
*/

#define MAX_ISIGNAL 128

static bool interrupt_raw(vm_t* vm, var_t* obj, const char* func_name, var_t* func, var_t* args) {
	while(vm->interrupted) { } // can not interrupt another interrupter.

	pthread_mutex_lock(&vm->interrupt_lock);
	if(vm->isignal_num >= MAX_ISIGNAL) {
		_err("Too many interrupt signals!\n");
		if(args != NULL)
			var_unref(vm, args, true);
		pthread_mutex_unlock(&vm->interrupt_lock);
		return false;
	}

	isignal_t* is = (isignal_t*)_malloc(sizeof(isignal_t));
	if(is == NULL) {
		_err("Interrupt signal input error!\n");
		if(args != NULL)
			var_unref(vm, args, true);
		pthread_mutex_unlock(&vm->interrupt_lock);
		return false;
	}

	is->next = NULL;
	is->prev = NULL;
	is->obj = var_ref(obj);

	if(func != NULL)
		is->handle_func = var_ref(func);
	else
		is->handle_func = NULL;

	if(func_name != NULL && func_name[0] != 0) 
		is->handle_func_name = str_new(func_name);
	else
		is->handle_func_name = NULL;

	if(args != NULL)
		is->args = var_ref(args);
	else
		is->args = NULL;

	if(vm->isignal_tail == NULL) {
		vm->isignal_head = is;
		vm->isignal_tail = is;
	}
	else {
		vm->isignal_tail->next = is;
		is->prev = vm->isignal_tail;
		vm->isignal_tail = is;
	}

	vm->isignal_num++;
	pthread_mutex_unlock(&vm->interrupt_lock);
	return true;
}

bool interrupt_by_name(vm_t* vm, var_t* obj, const char* func_name, var_t* args) {
	return interrupt_raw(vm, obj, func_name, NULL, args);
}

bool interrupt(vm_t* vm, var_t* obj, var_t* func, var_t* args) {
	return interrupt_raw(vm, obj, NULL, func, args);
}

void tryInterrupter(vm_t* vm) {
	if(vm->isignal_head == NULL || vm->interrupted) {
		return;
	}

	pthread_mutex_lock(&vm->interrupt_lock);
	vm->interrupted = true;

	isignal_t* sig = vm->isignal_head;
		
	var_t* func = NULL;

	while(sig != NULL) {
		if(sig->handle_func != NULL) {
			func = sig->handle_func;
			break;
		}
		else if(sig->handle_func_name != NULL) {
			//if func undefined yet, keep it and try next sig
			node_t* func_node = find_member(sig->obj, sig->handle_func_name->cstr);
			if(func_node != NULL && func_node->var->is_func) { 
				func = var_ref(func_node->var);
				break;
			}
		}
		sig = sig->next;
	}

	if(func == NULL || sig == NULL) {
		vm->interrupted = false;
		pthread_mutex_unlock(&vm->interrupt_lock);
		return;
	}

	//pop this sig from queue.
	if(sig->prev == NULL) 
		vm->isignal_head = sig->next;
	else 
		sig->prev->next = sig->next;

	if(sig->next == NULL)
		vm->isignal_tail = sig->prev;
	else
		sig->next->prev = sig->prev;

	var_t* ret = call_js_func(vm, sig->obj, func, sig->args);

	if(ret != NULL)
		var_unref(vm, ret, true);

	var_unref(vm, sig->obj, true);
	var_unref(vm, func, true);
	if(sig->handle_func_name != NULL)
		str_free(sig->handle_func_name);
	if(sig->args != NULL)
		var_unref(vm, sig->args, true);
	_free(sig);
	vm->isignal_num--;
	vm->interrupted = false;
	pthread_mutex_unlock(&vm->interrupt_lock);
}

#endif

/*****************/

node_t* vm_new_class(vm_t* vm, const char* cls) {
	node_t* cls_node = vm_load_node(vm, cls, true);
	cls_node->var->type = V_OBJECT;

	if(get_prototype(cls_node->var) == NULL) {
		if(strcmp(cls, "Object") == 0) {
			vm->var_Object = cls_node->var;
			add_prototype(vm, vm->var_Object, NULL);
		}
		else {
			add_prototype(vm, cls_node->var, vm->var_Object);
		}
	}

	return cls_node;
}

void vm_run(vm_t* vm) {
	//int32_t scDeep = vm->scopes.size;
	register PC code_size = vm->bc.cindex;
	register PC* code = vm->bc.code_buf;

	do {
		register PC ins = code[vm->pc++];
		register opr_code_t instr = OP(ins);
		register opr_code_t offset = ins & 0x0000FFFF;

		if(instr == INSTR_END)
			break;
		
		switch(instr) {
			case INSTR_JMP: 
			{
				vm->pc = vm->pc + offset - 1;
				break;
			}
			case INSTR_JMPB: 
			{
				vm->pc = vm->pc - offset - 1;
				break;
			}
			case INSTR_NJMP: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					if(v->type == V_UNDEF ||
							v->value == NULL ||
							*(int*)(v->value) == 0)
						vm->pc = vm->pc + offset - 1;
					var_unref(vm, v, true);
				}
				break;
			}
			case INSTR_LOAD: 
			{
				bool loaded = false;
				var_t* sc_var = vm_get_scope_var(vm, true);

				#ifdef MARIO_CACHE
				if((ins & INSTR_NEED_IMPROVE) != 0) { //try cached.
					node_t* n = vm->node_cache[offset].node;
					if(vm->node_cache[offset].sc_var == sc_var) { //cached
						vm_push_node(vm, n);
						loaded = true;
					}
					else {
						offset = vm->node_cache[offset].name_id;
					}
				}
				#endif

				if(!loaded) {
					if(offset == vm->this_strIndex) {
						var_t* thisV = vm_this_in_scopes(vm);
						if(thisV != NULL) {
							vm_push(vm, thisV);	
							loaded = true;	
						}
					}
				}

				if(!loaded) {
					const char* s = bc_getstr(&vm->bc, offset);
					node_t* n = vm_load_node(vm, s, true); //load variable, create if not exist.
					vm_push_node(vm, n);

					#ifdef MARIO_CACHE
					int cache_id = node_cache(vm, sc_var, n, offset);
					if(cache_id >= 0) {
						code[vm->pc-1] = INSTR_NEED_IMPROVE | ( INSTR_LOAD << 16 ) | cache_id;
					}
					#endif
				}
				break;
			}
			case INSTR_LES: 
			case INSTR_EQ: 
			case INSTR_NEQ: 
			case INSTR_TEQ:
			case INSTR_NTEQ:
			case INSTR_GRT: 
			case INSTR_LEQ: 
			case INSTR_GEQ: 
			{
				var_t* v2 = vm_pop2(vm);
				var_t* v1 = vm_pop2(vm);
				if(v1 != NULL && v2 != NULL) {
					compare(vm, instr, v1, v2);
					var_unref(vm, v1, true);
					var_unref(vm, v2, true);
				}
				break;
			}
			case INSTR_NIL: {	break; }
			case INSTR_BLOCK: 
			{
				scope_t* bl = vm_get_scope(vm);
				scope_t* sc = NULL;
				if(bl != NULL)
					sc = scope_new(var_new_block(), bl->pc);
				else
					sc = scope_new(var_new_block(), 0xFFFFFFFF);
				vm_push_scope(vm, sc);
				break;
			}
			case INSTR_BLOCK_END: 
			{
				uint32_t i;
				for(i=0; i<offset; i++) {
					vm_pop_scope(vm);
				}	
				break;
			}
			#ifdef MARIO_CACHE
			case INSTR_CACHE: 
			{	
				var_t* v = vm->var_cache[offset];
				vm_push(vm, v);
				break;
			}
			#endif
			case INSTR_TRUE: 
			{
				vm_push(vm, vm->var_true);
				break;
			}
			case INSTR_FALSE: 
			{
				vm_push(vm, vm->var_false);
				break;
			}
			case INSTR_UNDEF: 
			{
				var_t* v = var_new();	
				vm_push(vm, v);
				break;
			}
			case INSTR_POP: 
			{
				vm_pop(vm);
				break;
			}
			case INSTR_NEG: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					if(v->type == V_INT) {
						int n = *(int*)v->value;
						n = -n;
						vm_push(vm, var_new_int(n));
					}
					else if(v->type == V_FLOAT) {
						float n = *(float*)v->value;
						n = -n;
						vm_push(vm, var_new_float(n));
					}
					var_unref(vm, v, true);
				}
				break;
			}
			case INSTR_NOT: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					bool i = false;
					if(v->type == V_UNDEF || *(int*)v->value == 0)
						i = true;
					var_unref(vm, v, true);
					vm_push(vm, i ? vm->var_true:vm->var_false);
				}
				break;
			}
			case INSTR_AAND: 
			case INSTR_OOR: 
			{
				var_t* v2 = vm_pop2(vm);
				var_t* v1 = vm_pop2(vm);
				if(v1 != NULL && v2 != NULL) {
					bool r = false;
					int i1 = *(int*)v1->value;
					int i2 = *(int*)v2->value;

					if(instr == INSTR_AAND)
						r = (i1 != 0) && (i2 != 0);
					else
						r = (i1 != 0) || (i2 != 0);
					vm_push(vm, r ? vm->var_true:vm->var_false);

					var_unref(vm, v1, true);
					var_unref(vm, v2, true);
				}
				break;
			}
			case INSTR_PLUS: 
			case INSTR_RSHIFT: 
			case INSTR_LSHIFT: 
			case INSTR_AND: 
			case INSTR_OR: 
			case INSTR_PLUSEQ: 
			case INSTR_MULTIEQ: 
			case INSTR_DIVEQ: 
			case INSTR_MODEQ: 
			case INSTR_MINUS: 
			case INSTR_MINUSEQ: 
			case INSTR_DIV: 
			case INSTR_MULTI: 
			case INSTR_MOD:
			{
				var_t* v2 = vm_pop2(vm);
				var_t* v1 = vm_pop2(vm);
				if(v1 != NULL && v2 != NULL) {
					math_op(vm, instr, v1, v2);
					var_unref(vm, v1, true);
					var_unref(vm, v2, true);
				}
				break;
			}
			case INSTR_MMINUS_PRE: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					int *i = (int*)v->value;
					if(i != NULL) {
						(*i)--;
						if((ins & INSTR_NEED_IMPROVE) == 0) {
							if(OP(code[vm->pc]) != INSTR_POP) { 
								vm_push(vm, v);
							}
							else { code[vm->pc] = INSTR_NIL; code[vm->pc-1] |= INSTR_NEED_IMPROVE; }
						}
					}
					else {
						vm_push(vm, v);
					}
					var_unref(vm, v, true);
				}
				break;
			}
			case INSTR_MMINUS: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					int *i = (int*)v->value;
					if(i != NULL) {
						if((ins & INSTR_NEED_IMPROVE) == 0) {
							var_t* v2 = var_new_int(*i);
							if(OP(code[vm->pc]) != INSTR_POP) {
								vm_push(vm, v2);
							}
							else { 
								code[vm->pc] = INSTR_NIL; 
								code[vm->pc-1] |= INSTR_NEED_IMPROVE;
								var_unref(vm, v2, true);
							}
						}
						(*i)--;
					}
					else {
						vm_push(vm, v);
					}
					var_unref(vm, v, true);
				}
				break;
			}
			case INSTR_PPLUS_PRE: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					int *i = (int*)v->value;
					if(i != NULL) {
						(*i)++;

						if((ins & INSTR_NEED_IMPROVE) == 0) {
							if(OP(code[vm->pc]) != INSTR_POP) { 
								vm_push(vm, v);
							}
							else { code[vm->pc] = INSTR_NIL; code[vm->pc-1] |= INSTR_NEED_IMPROVE; }
						}
					}
					else {
						vm_push(vm, v);
					}
					var_unref(vm, v, true);
				}
				break;

			}
			case INSTR_PPLUS: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					int *i = (int*)v->value;
					if(i != NULL) {
						if((ins & INSTR_NEED_IMPROVE) == 0) {
							var_t* v2 = var_new_int(*i);
							if(OP(code[vm->pc]) != INSTR_POP) {
								vm_push(vm, v2);
							}
							else { 
								code[vm->pc] = INSTR_NIL;
								code[vm->pc-1] |= INSTR_NEED_IMPROVE; 
								var_unref(vm, v2, true);
							}
						}

						(*i)++;
					}
					else {
						vm_push(vm, v);
					}
					var_unref(vm, v, true);
				}
				break;
			}
			case INSTR_RETURN:  //return without value
			case INSTR_RETURNV: 
			{ //return with value
				scope_t* sc = vm_get_scope(vm);
				if(sc != NULL) {
					if(instr == INSTR_RETURN) {//return without value, push "this" to stack
						var_t* thisV = vm_this_in_scopes(vm);
						if(thisV != NULL)
							vm_push(vm, thisV);
						else
							vm_push(vm, var_new());
					}

					vm->pc = sc->pc;
					vm_pop_scope(vm);
				}
				return;
			}
			case INSTR_VAR:
			{
				const char* s = bc_getstr(&vm->bc, offset);
				node_t *node = vm_find(vm, s);
				if(node != NULL) { //find just in current scope
					_err("Warning: '");
					_err(s);
					_err("' has already existed!\n");
				}
				else {
					var_t* v = vm_get_scope_var(vm, true);
					if(v != NULL) {
						node = var_add(vm, v, s, NULL);
					}
				}
				break;
			}
			case INSTR_LET:
			case INSTR_CONST: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				var_t* v = vm_get_scope_var(vm, false);
				node_t *node = var_find(v, s);
				if(node != NULL) { //find just in current scope
					_err("Error: let '");
					_err(s);
					_err("' has already existed!\n");
					vm->pc = code_size; //exit.
				}
				else {
					node = var_add(vm, v, s, NULL);
					if(node != NULL && instr == INSTR_CONST)
						node->be_const = true;
				}
				break;
			}
			case INSTR_INT:
			{
				var_t* v = var_new_int((int)code[vm->pc++]);
				#ifdef MARIO_CACHE
				if(try_cache(vm, &code[vm->pc-2], v))
					code[vm->pc-1] = INSTR_NIL;
				#endif
				vm_push(vm, v);
				break;
			}
			case INSTR_INT_S:
			{
				var_t* v = var_new_int(offset);
				#ifdef MARIO_CACHE
				try_cache(vm, &code[vm->pc-1], v);
				#endif
				vm_push(vm, v);
				break;
			}
			case INSTR_FLOAT: 
			{
				var_t* v = var_new_float(*(float*)(&code[vm->pc++]));
				#ifdef MARIO_CACHE
				if(try_cache(vm, &code[vm->pc-2], v))
					code[vm->pc-1] = INSTR_NIL;
				#endif
				vm_push(vm, v);
				break;
			}
			case INSTR_STR: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				var_t* v = var_new_str(s);
				#ifdef MARIO_CACHE	
				try_cache(vm, &code[vm->pc-1], v);
				#endif
				vm_push(vm, v);
				break;
			}
			case INSTR_ASIGN: 
			{
				var_t* v = vm_pop2(vm);
				node_t* n = vm_pop2node(vm);
				if(v != NULL && n != NULL) {
					bool modi = (!n->be_const || n->var->type == V_UNDEF);
					var_unref(vm, n->var, true);
					if(modi) 
						node_replace(vm, n, v);
					else {
						_err("Can not change a const variable: '");
						_err(n->name);
						_err("'!\n");
					}
					var_unref(vm, v, true);

					if((ins & INSTR_NEED_IMPROVE) == 0) {
						if(OP(code[vm->pc]) != INSTR_POP) {
							vm_push(vm, n->var);
						}
						else { code[vm->pc] = INSTR_NIL; code[vm->pc-1] |= INSTR_NEED_IMPROVE; }
					}
				}
				break;
			}
			case INSTR_GET: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					do_get(vm, v, s);
					var_unref(vm, v, true);
				}
				else {
					vm_push(vm, var_new());
					_err("Error: can not find member '");
					_err(s);
					_err("'!\n");
				}
				break;
			}
			case INSTR_NEW: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				do_new(vm, s);
				break;
			}
			case INSTR_CALL: 
			case INSTR_CALLO: 
			{
				var_t* func = NULL;
				var_t* obj = NULL;
				bool unrefObj = false;
				const char* s = bc_getstr(&vm->bc, offset);
				str_t* name = str_new("");
				int arg_num = parse_func_name(s, name);
				
				if(instr == INSTR_CALLO) {
					obj = vm_stack_pick(vm, arg_num+1);
					unrefObj = true;
				}

				func = find_func(vm, obj, name->cstr);
				if(func != NULL && !func->is_func) { //constructor
					var_t* constr = var_find_var(func, CONSTRUCTOR);	
					if(constr == NULL) {
						var_t* protoV = get_prototype(func);
						if(protoV != NULL) 
							func = var_find_var(protoV, CONSTRUCTOR);	
						else
							func = NULL;
					}
					else {
						func = constr;
					}
				}

				if(func != NULL) {
					func_call(vm, obj, func, arg_num);
				}
				else {
					while(arg_num > 0) {
						vm_pop(vm);
						arg_num--;
					}
					vm_push(vm, var_new());
					_err("Error: can not find function '");
					_err(s);
					_err("'!\n");
				}
				str_free(name);

				if(unrefObj && obj != NULL)
					var_unref(vm, obj, true);

				//check and do interrupter.
				#ifdef MARIO_THREAD
				tryInterrupter(vm);
				#endif
				break;
			}
			case INSTR_MEMBER: 
			case INSTR_MEMBERN: 
			{
				const char* s = (instr == INSTR_MEMBER ? "" :  bc_getstr(&vm->bc, offset));

				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					var_t *var = vm_get_scope_var(vm, true);

					if(v->is_func) {
						func_t* func = (func_t*)v->value;
						func->owner = var;
					}
					if(var != NULL) {
						var_add(vm, var, s, v);
					}	

					var_unref(vm, v, true);
				}
				break;
			}

			case INSTR_FUNC: 
			case INSTR_FUNC_STC: 
			case INSTR_FUNC_GET: 
			case INSTR_FUNC_SET: 
			{
				var_t* v = func_def(vm, 
						(instr == INSTR_FUNC ? true:false),
						(instr == INSTR_FUNC_STC ? true:false));
				if(v != NULL)
					vm_push(vm, v);
				break;
			}
			case INSTR_OBJ:
			case INSTR_ARRAY: 
			{
				var_t* obj;
				if(instr == INSTR_OBJ) {
					obj = var_new_obj(NULL, NULL);
					var_add(vm, obj, PROTOTYPE, get_prototype(vm->var_Object));
				}
				else
					obj = var_new_array();
				scope_t* sc = scope_new(obj, 0xFFFFFFFF);
				vm_push_scope(vm, sc);
				break;
			}
			case INSTR_ARRAY_END: 
			case INSTR_OBJ_END: 
			{
				var_t* obj = vm_get_scope_var(vm, true);
				vm_push(vm, obj); //that actually means currentObj->ref() for push and unref for unasign.
				vm_pop_scope(vm);
				break;
			}
			case INSTR_ARRAY_AT: 
			{
				var_t* v2 = vm_pop2(vm);
				var_t* v1 = vm_pop2(vm);
				if(v1 != NULL && v2 != NULL) {
					int at = var_get_int(v2);
					var_unref(vm, v2, true);

					node_t* n = var_get(vm, v1, at);
					if(n != NULL) {
						vm_push_node(vm, n);
					}
					var_unref(vm, v1, true);
				}
				break;
			}
			case INSTR_CLASS: 
			{
				const char* s =  bc_getstr(&vm->bc, offset);
				node_t* n = vm_new_class(vm, s);
				var_t* protoV = get_prototype(n->var);
				//read extends
				ins = code[vm->pc];
				instr = ins >> 16;
				if(instr == INSTR_EXTENDS) {
					vm->pc++;
					offset = ins & 0x0000FFFF;
					s =  bc_getstr(&vm->bc, offset);
					doExtends(vm, protoV, s);
				}

				scope_t* sc = scope_new(protoV, 0xFFFFFFFF);
				vm_push_scope(vm, sc);
				break;
			}
			case INSTR_CLASS_END: 
			{
				var_t* var = vm_get_scope_var(vm, true);
				vm_push(vm, var);
				vm_pop_scope(vm);
				break;
			}
			case INSTR_TYPEOF: 
			{
				var_t* var = vm_pop2(vm);
				var_t* v = var_new_str(get_typeof(var));
				vm_push(vm, v);
				break;
			}
			/*
			case INSTR_THROW: {
				BC_var *var = reinterpret_cast<BC_var*>(pop2());
				exception = var->ref();

				// walk through scopes to find exception handle
				VMScope* sc = scope();
				while(sc != NULL) {
					PC target = bcode->getTryTarget(pc);
					if(target != ILLEGAL_PC) {
						pc = target;
						break;
					}
					pc = sc->pc;
					popScope();
					sc = scope();
				}
				if(sc == NULL) {
					ERR("uncaught exception:%s\n", exception->get_string().c_str());
					return;
				}

				break;
			}
			case INSTR_MOV_EXCP: {
				BC_node *node = reinterpret_cast<BC_node*>(pop2());
				node->replace(VAR(exception));
				exception = NULL;
				break;
			}
			*/
		}
	}
	while(vm->pc < code_size);
}

bool vm_load(vm_t* vm, const char* s) {
	/*
	if(vm->bc.cindex > 0) {
		vm->bc.cindex--;
	}
	*/
	vm->pc = vm->bc.cindex;
	return compile(&vm->bc, s);
}

bool vm_load_run(vm_t* vm, const char* s) {
	bool ret = false;
	if(vm_load(vm, s)) {
		vm_run(vm);
		ret = true;
	}
	return ret;
}

bool vm_load_run_native(vm_t* vm, const char* s) {
	bool ret = false;
	PC old = vm->pc;

	if(vm_load(vm, s)) {
		vm_run(vm);
		ret = true;
	}

	vm->pc = old;
	return ret;
}

typedef struct st_native_init {
	void (*func)(void*);
	void *data;
} native_init_t;


void vm_dump(vm_t* vm) {
#ifdef MARIO_DEBUG
	bc_dump(&vm->bc);
#endif
}

void vm_close(vm_t* vm) {
	vm->terminated = true;
	int i;
	for(i=0; i<vm->close_natives.size; i++) {
		native_init_t* it = (native_init_t*)array_get(&vm->close_natives, i);
		it->func(it->data);
	}
	array_clean(&vm->close_natives, NULL, NULL);	

	var_unref(vm, vm->root, true);
	var_unref(vm, vm->var_true, true);
	var_unref(vm, vm->var_false, true);


	#ifdef MARIO_THREAD
	pthread_mutex_destroy(&vm->interrupt_lock);
	#endif

	#ifdef MARIO_CACHE
	var_cache_free(vm);
	#endif

	array_clean(&vm->scopes, NULL, NULL);	
	array_clean(&vm->init_natives, NULL, NULL);	


	bc_release(&vm->bc);
	vm->stack_top = 0;

	if(vm->on_close != NULL)
		vm->on_close(vm);
	_free(vm);
}	

/** native extended functions.-----------------------------*/

void vm_reg_init(vm_t* vm, void (*func)(void*), void* data) {
	native_init_t* it = (native_init_t*)_malloc(sizeof(native_init_t));
	it->func = func;
	it->data = data;
	array_add(&vm->init_natives, it);
}

void vm_reg_close(vm_t* vm, void (*func)(void*), void* data) {
	native_init_t* it = (native_init_t*)_malloc(sizeof(native_init_t));
	it->func = func;
	it->data = data;
	array_add(&vm->close_natives, it);
}

node_t* vm_reg_var(vm_t* vm, const char* cls, const char* name, var_t* var, bool be_const) {
	var_t* cls_var = vm->root;
	if(cls[0] != 0) {
		node_t* clsnode = vm_new_class(vm, cls);
		cls_var = get_prototype(clsnode->var);
		//cls_var = clsnode->var;
	}

	node_t* node = var_add(vm, cls_var, name, var);
	node->be_const = be_const;
	return node;
}

node_t* vm_reg_native(vm_t* vm, const char* cls, const char* decl, native_func_t native, void* data) {
	var_t* cls_var = vm->root;
	if(cls[0] != 0) {
		node_t* cls_node = vm_new_class(vm, cls);
		cls_var = get_prototype(cls_node->var);
		//cls_var = cls_node->var;
	}

	str_t* name = str_new("");
	str_t* arg = str_new("");

	func_t* func = func_new();
	func->native = native;
	func->data = data;

	const char *off = decl;
	//read name
	while(*off != '(') { 
		if(*off != ' ') //skip spaces
			str_add(name, *off);
		off++; 
	}
	off++; 

	while(*off != 0) {
		if(*off == ',' || *off == ')') {
			if(arg->len > 0)
				array_add_buf(&func->args, arg->cstr, arg->len+1);
			str_reset(arg);
		}
		else if(*off != ' ') //skip spaces
			str_add(arg, *off);

		off++; 
	} 
	str_free(arg);

	var_t* var = var_new_func(vm, func);
	node_t* node = var_add(vm, cls_var, name->cstr, var);
	str_free(name);

	return node;
}

node_t* vm_reg_static(vm_t* vm, const char* cls, const char* decl, native_func_t native, void* data) {
	node_t* n = vm_reg_native(vm, cls, decl, native, data);
	func_t* func = var_get_func(n->var);
	func->is_static = true;
	return n;
}

const char* get_str(var_t* var, const char* name) {
	node_t* n = var_find(var, name);
	return n == NULL ? "" : var_get_str(n->var);
}

int get_int(var_t* var, const char* name) {
	node_t* n = var_find(var, name);
	return n == NULL ? 0 : var_get_int(n->var);
}

bool get_bool(var_t* var, const char* name) {
	node_t* n = var_find(var, name);
	return n == NULL ? false : var_get_bool(n->var);
}

float get_float(var_t* var, const char* name) {
	node_t* n = var_find(var, name);
	return n == NULL ? 0.0 : var_get_float(n->var);
}

var_t* get_obj(var_t* var, const char* name) {
	if(strcmp(name, THIS) == 0)
		return var;

	node_t* n = var_find(var, name);
	if(n == NULL)
		return NULL;
	return n->var;
}

var_t* get_obj_member(var_t* env, const char* name) {
	var_t* obj = get_obj(env, THIS);
	if(obj == NULL)
		return NULL;
	return var_find_var(obj, name);
}

var_t* set_obj_member(vm_t* vm, var_t* env, const char* name, var_t* var) {
	var_t* obj = get_obj(env, THIS);
	if(obj == NULL)
		return NULL;
	var_add(vm, obj, name, var);
	return var;
}

/**print string*/
var_t* native_print(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* v = var_find_var(env, "str");
	str_t* s = str_new("");
	var_to_str(vm, v, s);
	_out_func(s->cstr);
	str_free(s);
	return NULL;
}

var_t* native_println(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* v = var_find_var(env, "str");
	str_t* s = str_new("");
	var_to_str(vm, v, s);
	str_add(s, '\n');
	_out_func(s->cstr);
	str_free(s);
	return NULL;
}

/**yield */
var_t* native_yield(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data; (void)env;
	return NULL;
}

vm_t* vm_from(vm_t* vm) {
	vm_t* ret = vm_new();
  vm_init(ret, vm->on_init, vm->on_close);
	return ret;
}

vm_t* vm_new() {
	vm_t* vm = (vm_t*)_malloc(sizeof(vm_t));
	memset(vm, 0, sizeof(vm_t));

	vm->terminated = false;
	vm->pc = 0;
	vm->this_strIndex = 0;
	vm->stack_top = 0;

	bc_init(vm, &vm->bc);
	array_init(&vm->scopes);	

	vm->var_true = var_new_bool(true);
	var_ref(vm->var_true);
	vm->var_false = var_new_bool(false);
	var_ref(vm->var_false);

	#ifdef MARIO_CACHE
	var_cache_init(vm);
	node_cache_init(vm);
	#endif

	#ifdef MARIO_THREAD
	pthread_mutex_init(&vm->interrupt_lock, NULL);

	vm->isignal_head = NULL;
	vm->isignal_tail = NULL;
	vm->isignal_num = 0;
	vm->interrupted = false;

	#endif

	array_init(&vm->close_natives);	
	array_init(&vm->init_natives);	
	
	vm->root = var_new_obj(NULL, NULL);
	var_ref(vm->root);

	vm_new_class(vm, "Object");

	vm_reg_native(vm, "console", "log(str)", native_print, NULL);
	vm_reg_native(vm, "console", "ln(str)", native_println, NULL);
	vm_reg_native(vm, "", "yield()", native_yield, NULL);

	return vm;
}

void vm_init(vm_t* vm,
		void (*on_init)(struct st_vm* vm),
		void (*on_close)(struct st_vm* vm)) {
	vm->on_init = on_init;
	vm->on_close = on_close;
	
	if(vm->on_init != NULL)
		vm->on_init(vm);

	int i;
	for(i=0; i<vm->init_natives.size; i++) {
		native_init_t* it = (native_init_t*)array_get(&vm->init_natives, i);
		it->func(it->data);
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

