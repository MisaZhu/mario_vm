/**
very tiny js engine in single file.
*/

#ifndef MARIO_JS
#define MARIO_JS

#include <inttypes.h>
#include <string.h>

/*typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
*/

typedef int bool;
static const int true = 1;
static const int false = 0;


/**
memory functions.
-----------------------------
*/
#ifndef PRE_ALLOC
#include <stdlib.h>
	#define _malloc malloc
	#define _realloc realloc
	#define _free free
#else
/*TODO*/
#endif

typedef void (*free_func_t)(void* p);

/**
array functions.
-----------------------------
*/

#define ARRAY_BUF 16

typedef struct st_array {
	void** items;
	uint32_t max: 16;
	uint32_t size: 16;
} m_array_t;

void array_init(m_array_t* array) {
	array->items = NULL;
	array->max = 0;
	array->size = 0;
}

void* array_add(m_array_t* array, void* item) {
	int new_size = array->size + 1;
	if(array->max <= new_size) {
		new_size = array->size + ARRAY_BUF; /*ARRAY BUF for buffer*/
		array->items = (void**)_realloc(array->items, new_size*sizeof(void*));
		array->max = new_size;
	}

	array->items[array->size] = item;
	array->size++;
	array->items[array->size] = NULL;
	return item;
}

void* array_get(m_array_t* array, uint32_t index) {
	if(array->items == NULL || index >= array->size)
		return NULL;
	return array->items[index];
}

void array_clean(m_array_t* array, free_func_t fr) {
	if(array->items != NULL) {
		int i;
		for(i=0; i<array->size; i++) {
			void* p = array->items[i];
			if(p != NULL) {
				if(fr != NULL)
					fr(p);
				else
					_free(p);
			}
		}
		_free(array->items);
		array->items = NULL;
	}
	array->max = array->size = 0;
}

/**
str functions.
-----------------------------
*/

#define STR_BUF 16

typedef struct st_str {
	char* cstr;
	uint32_t max: 16;
	uint32_t len: 16;
} str_t;

void str_init(str_t* str) {
	str->cstr = NULL;
	str->max = 0;
	str->len = 0;
}

void str_reset(str_t* str) {
	if(str->cstr == NULL) {
		str->cstr = (char*)_malloc(STR_BUF);
		str->max = STR_BUF;
	}

	str->cstr[0] = 0;
	str->len = 0;	
}

char* str_cpy(str_t* str, const char* src) {
	if(src == NULL || src[0] == 0) {
		str_reset(str);
		return str->cstr;
	}

	int len = strlen(src);
	int new_size = len;
	if(str->max <= new_size) {
		new_size = len + STR_BUF; /*STR BUF for buffer*/
		str->cstr = (char*)_realloc(str->cstr, new_size);
		str->max = new_size;
	}

	strcpy(str->cstr, src);
	str->len = len;
	return str->cstr;
}

char* str_append(str_t* str, const char* src) {
	if(src == NULL || src[0] == 0) {
		return str->cstr;
	}

	int len = strlen(src);
	int new_size = str->len + len;
	if(str->max <= new_size) {
		new_size = str->len + len + STR_BUF; /*STR BUF for buffer*/
		str->cstr = (char*)_realloc(str->cstr, new_size);
		str->max = new_size;
	}

	strcpy(str->cstr + str->len, src);
	str->len = str->len + len;
	return str->cstr;
}

char* str_add(str_t* str, char c) {
	int new_size = str->len + 1;
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

void str_release(str_t* str) {
	if(str->cstr != NULL) {
		_free(str->cstr);
		str->cstr = NULL;
	}
	str->max = str->len = 0;
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
			strncpy(p, str, i);
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

/**
stack functions.
-----------------------------
*/
typedef struct st_stack {
	struct st_stack* next;
	void* data;
} m_stack_t;

/**push new item to stack and return it as the top of stack
if return the input top, means pushing failed.
*/
m_stack_t* stack_push(m_stack_t* top, void* data) {
	m_stack_t *s = (m_stack_t*)_malloc(sizeof(m_stack_t));
	if(s == NULL) /*malloc error, push nothing.*/
		return top;
	
	s->data = data;
	s->next = top;
	return s;	
}

/**return the top of stack and pop it up.*/
m_stack_t* stack_pop(m_stack_t** stack) {
	m_stack_t *s = *stack;
	*stack = (*stack)->next;
	return s;
}

void stack_free(m_stack_t* s, free_func_t fr) {
	if(s == NULL)
		return;

	if(s->data != NULL) { /*free stack item data*/
		if(fr != NULL)
			fr(s->data);
		else
			_free(s->data);
	}
	_free(s); /*free stack item*/
}

/**
Script Lex.
-----------------------------
*/

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
  LEX_R_FUNCTION,
  LEX_R_CLASS,
  LEX_R_EXTENDS,
  LEX_R_RETURN,
  LEX_R_VAR,
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

	int32_t dataPos;
	int32_t dataStart, dataEnd;
	char currCh, nextCh;

	LEX_TYPES tk;
	str_t tkStr;
	int32_t tkStart, tkEnd, tkLastEnd;
} lex_t;

static unsigned char cmap[256]={
	//+0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,//00
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//10
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//20
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0,//30
	0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,//40
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 4,//50
	0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,//60
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0,//70
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//80
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//90
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//A0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//B0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//C0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//D0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//E0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//F0
}; 

bool is_whitespace(unsigned char ch) {
	if((cmap[ch]&1) == 0)
		return false;
	return true;
}

bool is_numeric(unsigned char ch) {
	if((cmap[ch]&2) == 0)
		return false;
	return true;
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
	if((cmap[ch]&4) == 0)
		return false;
	return true;
}

char* oneLine(const char *s, int ptr,int end) {
	uint32_t cnt = 0;
	static str_t work;
	str_init(&work);

	if( end < ptr ){
		ptr = end;
	}
	if( ptr>0) ptr--;
	if( ptr>0) ptr--;
	while( ptr > 0  && s[ptr] != '\n'){
		ptr--;
	}
	//ptr++;
	while( s[ptr] && s[ptr] != '\n' && cnt < sizeof( work )-2 ){
		str_add(&work, s[ptr]);
		ptr++;
	}
	return work.cstr;
}

/*
std::string getJSString(const std::string &str) {
	std::string nStr = str;
	for (size_t i=0;i<nStr.size();i++) {
		const char *replaceWith = "";
		bool replace = true;

		switch (nStr[i]) {
			case '\\': replaceWith = "\\\\"; break;
			case '\n': replaceWith = "\\n"; break;
			case '\r': replaceWith = "\\r"; break;
			case '\a': replaceWith = "\\a"; break;
			case '"':  replaceWith = "\\\""; break;
			default: {
								 int nCh = ((int)nStr[i]) &0xFF;
								 if (nCh<32 || nCh>127) {
									 char bytes[5];
									 snprintf(bytes, 5, "\\x%02X", nCh);
									 replaceWith = bytes;
								 } else {
									 replace=false;
								 }
							 }
		}

		if (replace) {
			nStr = nStr.substr(0, i) + replaceWith + nStr.substr(i+1);
			i += strlen(replaceWith)-1;
		}
	}
	return "\"" + nStr + "\"";
}
*/

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
	lex->currCh = lex->nextCh;
	if (lex->dataPos < lex->dataEnd){
		lex->nextCh = lex->data[lex->dataPos];
	}else{
		lex->nextCh = 0;
	}
	lex->dataPos++;
}

void lex_get_next_token(lex_t* lex) {
	lex->tk = LEX_EOF;
	str_reset(&lex->tkStr);

	while (lex->currCh && is_whitespace(lex->currCh)){
		lex_get_nextch(lex);
	}
	// newline comments
	if ((lex->currCh=='/' && lex->nextCh=='/') || (lex->currCh=='#' && lex->nextCh=='!')) {
		while (lex->currCh && lex->currCh!='\n'){
			lex_get_nextch(lex);
		}
		lex_get_nextch(lex);
		lex_get_next_token(lex);
		return;
	}
	// block comments
	if (lex->currCh=='/' && lex->nextCh=='*') {
		while (lex->currCh && (lex->currCh!='*' || lex->nextCh!='/')) {
			lex_get_nextch(lex);
		}
		lex_get_nextch(lex);
		lex_get_nextch(lex);
		lex_get_next_token(lex);
		return;
	}
	// record beginning of this token(pre-read 2 chars );
	lex->tkStart = lex->dataPos-2;
	// tokens
	if (is_alpha(lex->currCh)) { //  IDs
		while (is_alpha(lex->currCh) || is_numeric(lex->currCh)) {
			str_add(&lex->tkStr, lex->currCh);
			lex_get_nextch(lex);
		}
		lex->tk = LEX_ID;
		if (strcmp(lex->tkStr.cstr, "if") == 0)        lex->tk = LEX_R_IF;
		else if (strcmp(lex->tkStr.cstr, "else") == 0)      lex->tk = LEX_R_ELSE;
		else if (strcmp(lex->tkStr.cstr, "do") == 0)        lex->tk = LEX_R_DO;
		else if (strcmp(lex->tkStr.cstr, "while") == 0)    lex->tk = LEX_R_WHILE;
		else if (strcmp(lex->tkStr.cstr, "import") == 0)  lex->tk = LEX_R_INCLUDE;
		else if (strcmp(lex->tkStr.cstr, "for") == 0)     lex->tk = LEX_R_FOR;
		else if (strcmp(lex->tkStr.cstr, "break") == 0)    lex->tk = LEX_R_BREAK;
		else if (strcmp(lex->tkStr.cstr, "continue") == 0)  lex->tk = LEX_R_CONTINUE;
		else if (strcmp(lex->tkStr.cstr, "function") == 0)  lex->tk = LEX_R_FUNCTION;
		else if (strcmp(lex->tkStr.cstr, "class") ==0) 		 lex->tk = LEX_R_CLASS;
		else if (strcmp(lex->tkStr.cstr, "extends") == 0) 	 lex->tk = LEX_R_EXTENDS;
		else if (strcmp(lex->tkStr.cstr, "return") == 0)   lex->tk = LEX_R_RETURN;
		else if (strcmp(lex->tkStr.cstr, "var")  == 0)      lex->tk = LEX_R_VAR;
		else if (strcmp(lex->tkStr.cstr, "const") == 0)     lex->tk = LEX_R_CONST;
		else if (strcmp(lex->tkStr.cstr, "true") == 0)      lex->tk = LEX_R_TRUE;
		else if (strcmp(lex->tkStr.cstr, "false") == 0)     lex->tk = LEX_R_FALSE;
		else if (strcmp(lex->tkStr.cstr, "null") == 0)      lex->tk = LEX_R_NULL;
		else if (strcmp(lex->tkStr.cstr, "undefined") == 0) lex->tk = LEX_R_UNDEFINED;
		else if (strcmp(lex->tkStr.cstr, "new") == 0)       lex->tk = LEX_R_NEW;
		else if (strcmp(lex->tkStr.cstr, "typeof") == 0)       lex->tk = LEX_R_TYPEOF;
		else if (strcmp(lex->tkStr.cstr, "throw") == 0)     lex->tk = LEX_R_THROW;
		else if (strcmp(lex->tkStr.cstr, "try") == 0)    	 lex->tk = LEX_R_TRY;
		else if (strcmp(lex->tkStr.cstr, "catch") == 0)     lex->tk = LEX_R_CATCH;
	} else if (is_numeric(lex->currCh)) { // Numbers
		bool isHex = false;
		if (lex->currCh=='0') {
			str_add(&lex->tkStr, lex->currCh);
			lex_get_nextch(lex);
		}
		if (lex->currCh=='x') {
			isHex = true;
			str_add(&lex->tkStr, lex->currCh);
			lex_get_nextch(lex);
		}
		lex->tk = LEX_INT;
		while (is_numeric(lex->currCh) || (isHex && is_hexadecimal(lex->currCh))) {
			str_add(&lex->tkStr, lex->currCh);
			lex_get_nextch(lex);
		}
		if (!isHex && lex->currCh=='.') {
			lex->tk = LEX_FLOAT;
			str_add(&lex->tkStr, '.');
			lex_get_nextch(lex);
			while (is_numeric(lex->currCh)) {
				str_add(&lex->tkStr, lex->currCh);
				lex_get_nextch(lex);
			}
		}
		// do fancy e-style floating point
		if (!isHex && (lex->currCh=='e'||lex->currCh=='E')) {
			lex->tk = LEX_FLOAT;
			str_add(&lex->tkStr, lex->currCh);
			lex_get_nextch(lex);
			if (lex->currCh=='-') {
				str_add(&lex->tkStr, lex->currCh);
				lex_get_nextch(lex);
			}
			while (is_numeric(lex->currCh)) {
				str_add(&lex->tkStr, lex->currCh);
				lex_get_nextch(lex);
			}
		}
	} else if (lex->currCh=='"') {
		// strings...
		lex_get_nextch(lex);
		while (lex->currCh && lex->currCh!='"') {
			if (lex->currCh == '\\') {
				lex_get_nextch(lex);
				switch (lex->currCh) {
					case 'n' : str_add(&lex->tkStr, '\n'); break;
					case 'r' : str_add(&lex->tkStr, '\r'); break;
					case 't' : str_add(&lex->tkStr, '\t'); break;
					case '"' : str_add(&lex->tkStr, '\"'); break;
					case '\\' : str_add(&lex->tkStr, '\\'); break;
					default: str_add(&lex->tkStr, lex->currCh);
				}
			} else {
				str_add(&lex->tkStr, lex->currCh);
			}
			lex_get_nextch(lex);
		}
		lex_get_nextch(lex);
		lex->tk = LEX_STR;
	} else if (lex->currCh=='\'') {
		// strings again...
		lex_get_nextch(lex);
		while (lex->currCh && lex->currCh!='\'') {
			if (lex->currCh == '\\') {
				lex_get_nextch(lex);
				switch (lex->currCh) {
					case 'n' : str_add(&lex->tkStr, '\n'); break;
					case 'a' : str_add(&lex->tkStr, '\a'); break;
					case 'r' : str_add(&lex->tkStr, '\r'); break;
					case 't' : str_add(&lex->tkStr, '\t'); break;
					case '\'' : str_add(&lex->tkStr, '\''); break;
					case '\\' : str_add(&lex->tkStr, '\\'); break;
					case 'x' : { // hex digits
											 char buf[3] = "??";
											 lex_get_nextch(lex);
											 buf[0] = lex->currCh;
											 lex_get_nextch(lex);
											 buf[1] = lex->currCh;
											 str_add(&lex->tkStr, (char)strtol(buf,0,16));
										 } break;
					default: if (lex->currCh>='0' && lex->currCh<='7') {
										 // octal digits
										 char buf[4] = "???";
										 buf[0] = lex->currCh;
										 lex_get_nextch(lex);
										 buf[1] = lex->currCh;
										 lex_get_nextch(lex);
										 buf[2] = lex->currCh;
										 str_add(&lex->tkStr, (char)strtol(buf,0,8));
									 } else
										 str_add(&lex->tkStr, lex->currCh);
				}
			} else {
				str_add(&lex->tkStr, lex->currCh);
			}
			lex_get_nextch(lex);
		}
		lex_get_nextch(lex);
		lex->tk = LEX_STR;
	} else {
		// single chars
		lex->tk = (LEX_TYPES)lex->currCh;
		if (lex->currCh) 
			lex_get_nextch(lex);
		if (lex->tk=='=' && lex->currCh=='=') { // ==
			lex->tk = LEX_EQUAL;
			lex_get_nextch(lex);
			if (lex->currCh=='=') { // ===
				lex->tk = LEX_TYPEEQUAL;
				lex_get_nextch(lex);
			}
		} else if (lex->tk=='!' && lex->currCh=='=') { // !=
			lex->tk = LEX_NEQUAL;
			lex_get_nextch(lex);
			if (lex->currCh=='=') { // !==
				lex->tk = LEX_NTYPEEQUAL;
				lex_get_nextch(lex);
			}
		} else if (lex->tk=='<' && lex->currCh=='=') {
			lex->tk = LEX_LEQUAL;
			lex_get_nextch(lex);
		} else if (lex->tk=='<' && lex->currCh=='<') {
			lex->tk = LEX_LSHIFT;
			lex_get_nextch(lex);
			if (lex->currCh=='=') { // <<=
				lex->tk = LEX_LSHIFTEQUAL;
				lex_get_nextch(lex);
			}
		} else if (lex->tk=='>' && lex->currCh=='=') {
			lex->tk = LEX_GEQUAL;
			lex_get_nextch(lex);
		} else if (lex->tk=='>' && lex->currCh=='>') {
			lex->tk = LEX_RSHIFT;
			lex_get_nextch(lex);
			if (lex->currCh=='=') { // >>=
				lex->tk = LEX_RSHIFTEQUAL;
				lex_get_nextch(lex);
			} else if (lex->currCh=='>') { // >>>
				lex->tk = LEX_RSHIFTUNSIGNED;
				lex_get_nextch(lex);
			}
		}  else if (lex->tk=='+' && lex->currCh=='=') {
			lex->tk = LEX_PLUSEQUAL;
			lex_get_nextch(lex);
		}  else if (lex->tk=='-' && lex->currCh=='=') {
			lex->tk = LEX_MINUSEQUAL;
			lex_get_nextch(lex);
		}  else if (lex->tk=='*' && lex->currCh=='=') {
			lex->tk = LEX_MULTIEQUAL;
			lex_get_nextch(lex);
		}  else if (lex->tk=='/' && lex->currCh=='=') {
			lex->tk = LEX_DIVEQUAL;
			lex_get_nextch(lex);
		}  else if (lex->tk=='%' && lex->currCh=='=') {
			lex->tk = LEX_MODEQUAL;
			lex_get_nextch(lex);
		}  else if (lex->tk=='+' && lex->currCh=='+') {
			lex->tk = LEX_PLUSPLUS;
			lex_get_nextch(lex);
		}  else if (lex->tk=='-' && lex->currCh=='-') {
			lex->tk = LEX_MINUSMINUS;
			lex_get_nextch(lex);
		} else if (lex->tk=='&' && lex->currCh=='=') {
			lex->tk = LEX_ANDEQUAL;
			lex_get_nextch(lex);
		} else if (lex->tk=='&' && lex->currCh=='&') {
			lex->tk = LEX_ANDAND;
			lex_get_nextch(lex);
		} else if (lex->tk=='|' && lex->currCh=='=') {
			lex->tk = LEX_OREQUAL;
			lex_get_nextch(lex);
		} else if (lex->tk=='|' && lex->currCh=='|') {
			lex->tk = LEX_OROR;
			lex_get_nextch(lex);
		} else if (lex->tk=='^' && lex->currCh=='=') {
			lex->tk = LEX_XOREQUAL;
			lex_get_nextch(lex);
		}
	}
	/* This isn't quite right yet */
	lex->tkLastEnd = lex->tkEnd;
	lex->tkEnd = lex->dataPos-3;
}

void lex_reset(lex_t* lex) {
	lex->dataPos = lex->dataStart;
	lex->tkStart   = 0;
	lex->tkEnd     = 0;
	lex->tkLastEnd = 0;
	lex->tk  = LEX_EOF;
	str_reset(&lex->tkStr);
	lex_get_nextch(lex);
	lex_get_nextch(lex);
	lex_get_next_token(lex);
}

void lex_init(lex_t * lex, const char* input) {
	lex->data = input;
	lex->dataStart = 0;
	lex->dataEnd = strlen(lex->data);
	str_init(&lex->tkStr);
	lex_reset(lex);
}

void lex_release(lex_t* lex) {
	str_release(&lex->tkStr);
}

bool lex_chkread(lex_t* lex, int expected_tk) {
	if (lex->tk != expected_tk) {
		/*TODO error*/
		return false;
	}
	lex_get_next_token(lex);
	return true;
}

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
		case LEX_R_FUNCTION     : return "function";
		case LEX_R_CLASS     		: return "class";
		case LEX_R_EXTENDS   		: return "extends";
		case LEX_R_RETURN       : return "return";
		case LEX_R_CONST        : return "CONST";
		case LEX_R_VAR          : return "var";
		case LEX_R_TRUE         : return "true";
		case LEX_R_FALSE        : return "false";
		case LEX_R_NULL         : return "null";
		case LEX_R_UNDEFINED    : return "undefined";
		case LEX_R_NEW          : return "new";
		case LEX_R_INCLUDE      : return "import";
	}
	return "?[UNKNOW]";
}

/*
void CScriptLex::getPosition(int* line, int *col, int pos) {
	if (pos<0) pos=tkLastEnd;
	int l = 1;
	int c  = 1;
	for (int i=0;i<pos;i++) {
		char ch;
		if (i < dataEnd){
			ch = data[i];
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

std::string CScriptLex::getPosition(int pos) {
	int line = 1;
	int col;

	getPosition(&line, &col, pos);
	std::stringstream ss;
	ss << "(line: " << line << ", col: " << col << ")";
	return ss.str();
}
*/

/**
Script Var
-----------------------------
*/
const static uint8_t UNDEF = 0;
const static uint8_t INT = 1;
const static uint8_t FLOAT = 2;
const static uint8_t STRING = 3;
const static uint8_t POINT = 4;
const static uint8_t FUNC = 5;
const static uint8_t NFUNC = 6;
const static uint8_t OBJECT = 7;
const static uint8_t CLASS = 8;
const static uint8_t BYTES = 9;
const static uint8_t ARRAY = 10;

typedef struct st_var {
	int32_t refs:24;
	int32_t type:8;

	void* value;

	m_array_t* children;
} var_t;

/**
Interpretor
-----------------------------
*/

bool statement(lex_t*);
bool factor(lex_t*);
bool base(lex_t*);

int callFunc(lex_t* l) {
	lex_chkread(l, '(');
	int argNum = 0;
	while(true) {
		//PC pc1 = bytecode->getPC();
		if(!base(l))
			return -1;
		//PC pc2 = bytecode->getPC();
		//if(pc2 > pc1) //not empty, means valid arguemnt.
		argNum++;

		if (l->tk!=')')
			lex_chkread(l, ',');	
		else
			break;
	}
	lex_chkread(l, ')');
	return argNum;
}

bool factor(lex_t* l) {
	if (l->tk=='(') {
		lex_chkread(l, '(');
		base(l);
		lex_chkread(l, ')');
	}
	else if (l->tk==LEX_R_TRUE) {
		lex_chkread(l, LEX_R_TRUE);
		//bytecode->gen(INSTR_TRUE);
	}
	else if (l->tk==LEX_R_FALSE) {
		lex_chkread(l, LEX_R_FALSE);
		//bytecode->gen(INSTR_FALSE);
	}
	else if (l->tk==LEX_R_NULL) {
		lex_chkread(l, LEX_R_NULL);
		//bytecode->gen(INSTR_NULL);
	}
	else if (l->tk==LEX_R_UNDEFINED) {
		lex_chkread(l, LEX_R_UNDEFINED);
		//bytecode->gen(INSTR_UNDEF);
	}
	else if (l->tk==LEX_INT) {
		//bytecode->gen(INSTR_INT, l->tkStr);
		int i = str_to_int(l->tkStr.cstr);
		lex_chkread(l, LEX_INT);
	}
	else if (l->tk==LEX_FLOAT) {
		//bytecode->gen(INSTR_FLOAT, l->tkStr);
		float f = str_to_float(l->tkStr.cstr);
		lex_chkread(l, LEX_FLOAT);
	}
	else if (l->tk==LEX_STR) {
		//bytecode->gen(INSTR_STR, l->tkStr);
		lex_chkread(l, LEX_STR);
	}
	else if(l->tk==LEX_R_FUNCTION) {
		lex_chkread(l, LEX_R_FUNCTION);
		//string name;
		//defFunc(name);
	}
	else if(l->tk==LEX_R_CLASS) {
		//defClass();
	}
	/*else if (l->tk==LEX_R_NEW) {
		// new -> create a new object
		lex_chkread(l, LEX_R_NEW);
		std::string className = l->tkStr;
		lex_chkread(l, LEX_ID);
		if (l->tk == '(') {
			//lex_chkread(l, '(');
			int argNum;
			callFunc(argNum);
			//lex_chkread(l, ')');
			if(argNum > 0)
				className = className + "$" + StringUtil::from(argNum);
			bytecode->gen(INSTR_NEW, className);
		}
	}
	*/

	if (l->tk=='{') {
		// JSON-style object definition
		lex_chkread(l, '{');
		//bytecode->gen(INSTR_OBJ);
		while (l->tk != '}') {
			//string id = l->tkStr;
			// we only allow strings or IDs on the left hand side of an initialisation
			if (l->tk==LEX_STR) 
				lex_chkread(l, LEX_STR);
			else 
				lex_chkread(l, LEX_ID);
			lex_chkread(l, ':');
			base(l);
			//bytecode->gen(INSTR_MEMBERN, id);
			// no need to clean here, as it will definitely be used
			if (l->tk != '}') 
				lex_chkread(l, ',');
		}
		//bytecode->gen(INSTR_OBJ_END);
		lex_chkread(l, '}');
	}
	else if(l->tk==LEX_ID) {
		str_t name;
		str_init(&name);
		str_cpy(&name, l->tkStr.cstr);
		lex_chkread(l, LEX_ID);

		m_array_t names;
		array_init(&names);

		bool load = true;
		while (l->tk=='(' || l->tk=='.' || l->tk=='[') {
			if (l->tk=='(') { // ------------------------------------- Function Call
				str_split(name.cstr, '.', &names);
				str_reset(&name);

				int sz = (int)(names.size-1);
				str_t s;
				str_init(&s);
				str_cpy(&s, (const char*)names.items[sz]);
					
				if(sz == 0 && load) {
					//bytecode->gen(INSTR_LOAD, "this");	
					int argNum = callFunc(l);
					//if(argNum > 0)
					//	s = s + "$" + StringUtil::from(argNum);
					//bytecode->gen(INSTR_CALL, s);	
				}
				else {
					int i;
					for(i=0; i<sz; i++) {
						//bytecode->gen(load ? INSTR_LOAD:INSTR_GET, names[i]);	
						load = false;
					}
					int argNum = callFunc(l);
					//if(argNum > 0)
					//	s = s + "$" + StringUtil::from(argNum);
					//bytecode->gen(INSTR_CALLO, s);	
				}
				load = false;
				array_clean(&names, NULL);
				str_release(&s);
			} 
			else if (l->tk == '.') { // ------------------------------------- Record Access
				lex_chkread(l, '.');
				if(name.len == 0)
					str_cpy(&name, l->tkStr.cstr);
				else {
					str_append(&name, ".");
					str_append(&name, l->tkStr.cstr);
				}
				lex_chkread(l, LEX_ID);
			} 
			else { // ------------------------------------- Array Access
				int i;
				int sz;

				str_split(name.cstr, '.', &names);
				str_reset(&name);
				sz = names.size;
				for(i=0; i<sz; i++) {
				//	bytecode->gen(load ? INSTR_LOAD:INSTR_GET, names[i]);	
					load = false;
				}

				lex_chkread(l, '[');
				base(l);
				lex_chkread(l, ']');
				//bytecode->gen(INSTR_ARRAY_AT);
			} 
		}
		if(name.len > 0) {
			int i, sz;
			str_split(name.cstr, '.', &names);
			str_reset(&name);
			sz = names.size;
			for(i=0; i<sz; i++) {
				//bytecode->gen(load ? INSTR_LOAD:INSTR_GET, names[i]);	
				load = false;
			}
		}
		str_release(&name);
	}
	else if (l->tk=='[') {
		// JSON-style array 
		lex_chkread(l, '[');
		//bytecode->gen(INSTR_ARRAY);
		while (l->tk != ']') {
			base(l);
		//	bytecode->gen(INSTR_MEMBER);
			if (l->tk != ']') 
				lex_chkread(l, ',');
		}
		lex_chkread(l, ']');
		//bytecode->gen(INSTR_ARRAY_END);
	}

	return true;
}

bool unary(lex_t* l) {
	//OprCode instr = INSTR_END;
	if (l->tk == '!') {
		lex_chkread(l, '!');
	//	instr = INSTR_NOT;
	} else if(l->tk == LEX_R_TYPEOF) {
		lex_chkread(l, LEX_R_TYPEOF);
	//	instr = INSTR_TYPEOF;
	}

	if(!factor(l))
		return false;

	//if(instr != INSTR_END) {
	//	bytecode->gen(instr);
	//}
	return true;	
}

bool term(lex_t* l) {
	if(!unary(l))
		return false;

	while (l->tk=='*' || l->tk=='/' || l->tk=='%') {
		LEX_TYPES op = l->tk;
		lex_chkread(l, l->tk);
		unary(l);

		if(op == '*') {
		//	bytecode->gen(INSTR_MULTI);
		}
		else if(op == '/') {
		//	bytecode->gen(INSTR_DIV);
		}
		else {
		//	bytecode->gen(INSTR_MOD);
		}
	}

	return true;	
}

bool expr(lex_t* l) {
	LEX_TYPES pre = l->tk;

	if (l->tk=='-') {
		lex_chkread(l, '-');
	}
	else if(l->tk==LEX_PLUSPLUS) {
		lex_chkread(l, LEX_PLUSPLUS);
	}
	else if(l->tk==LEX_MINUSMINUS) {
		lex_chkread(l, LEX_MINUSMINUS);
	}

	if(!term(l))
		return false;

	if (pre == '-') {
	//	bytecode->gen(INSTR_NEG);
	}
	else if(pre==LEX_PLUSPLUS) {
	//	bytecode->gen(INSTR_PPLUS_PRE);
	}
	else if(pre==LEX_MINUSMINUS) {
	//	bytecode->gen(INSTR_MMINUS_PRE);
	}

	while (l->tk=='+' || l->tk=='-' ||
			l->tk==LEX_PLUSPLUS || l->tk==LEX_MINUSMINUS) {
		int op = l->tk;
		lex_chkread(l, l->tk);
		if (op==LEX_PLUSPLUS) {
		//	bytecode->gen(INSTR_PPLUS);
		}
		else if(op==LEX_MINUSMINUS) {
		//	bytecode->gen(INSTR_MMINUS);
		}
		else {
			if(!term(l))
				return false;
			if(op== '+') {
			//	bytecode->gen(INSTR_PLUS);
			}
			else if(op=='-') {
			//	bytecode->gen(INSTR_MINUS);
			}
		}
	}

	return true;	
}

bool shift(lex_t* l) {
	if(!expr(l))
		return false;

	if (l->tk==LEX_LSHIFT || l->tk==LEX_RSHIFT || l->tk==LEX_RSHIFTUNSIGNED) {
		int op = l->tk;
		lex_chkread(l, op);
		if(!base(l))
			return false;

		if (op==LEX_LSHIFT) {
		//	bytecode->gen(INSTR_LSHIFT);
		}
		else if (op==LEX_RSHIFT) {
		//	bytecode->gen(INSTR_RSHIFT);
		}
		else {
		//	bytecode->gen(INSTR_URSHIFT);
		}
	}
	return true;	
}

bool condition(lex_t *l) {
	if(!shift(l))
		return false;

	while (l->tk==LEX_EQUAL || l->tk==LEX_NEQUAL ||
			l->tk==LEX_TYPEEQUAL || l->tk==LEX_NTYPEEQUAL ||
			l->tk==LEX_LEQUAL || l->tk==LEX_GEQUAL ||
			l->tk=='<' || l->tk=='>') {
		int op = l->tk;
		lex_chkread(l, l->tk);
		if(!shift(l))
			return false;

		if(op == LEX_EQUAL) {
		//	bytecode->gen(INSTR_EQ);
		}
		else if(op == LEX_NEQUAL) {
		//	bytecode->gen(INSTR_NEQ);
		}
		else if(op == LEX_TYPEEQUAL) {
		//	bytecode->gen(INSTR_TEQ);
		}
		else if(op == LEX_NTYPEEQUAL) {
		//	bytecode->gen(INSTR_NTEQ);
		}
		else if(op == LEX_LEQUAL) {
		//	bytecode->gen(INSTR_LEQ);
		}
		else if(op == LEX_GEQUAL) {
		//	bytecode->gen(INSTR_GEQ);
		}
		else if(op == '>') {
		//	bytecode->gen(INSTR_GRT);
		}
		else if(op == '<') {
		//	bytecode->gen(INSTR_LES);
		}
	}

	return true;	
}

bool logic(lex_t* l) {
	if(!condition(l))
		return false;

	while (l->tk=='&' || l->tk=='|' || l->tk=='^' || l->tk==LEX_ANDAND || l->tk==LEX_OROR) {
		int op = l->tk;
		lex_chkread(l, l->tk);
		if(!condition(l))
			return false;

		if (op==LEX_ANDAND) {
		//	bytecode->gen(INSTR_AAND);
		} 
		else if (op==LEX_OROR) {
		//	bytecode->gen(INSTR_OOR);
		}
		else if (op=='|') {
		//	bytecode->gen(INSTR_OR);
		}
		else if (op=='&') {
		//	bytecode->gen(INSTR_AND);
		}
		else if (op=='^') {
		//	bytecode->gen(INSTR_XOR);
		}
	}
	return true;	
}


bool ternary(lex_t *l) {
	if(!logic(l))
		return false;
	
	if (l->tk=='?') {
	/*
		PC pc1 = bytecode->reserve(); //keep for jump
		lex_chkread(l, '?');
		base(); //first choice
		PC pc2 = bytecode->reserve(); //keep for jump
		lex_chkread(l, ':');
		bytecode->setInstr(pc1, INSTR_NJMP);
		base(); //second choice
		bytecode->setInstr(pc2, INSTR_JMP);
		*/
	} 
	return true;	
}

bool base(lex_t* l) {
	if(!ternary(l))
		return false;

	if (l->tk=='=' || 
			l->tk==LEX_PLUSEQUAL ||
			l->tk==LEX_MULTIEQUAL ||
			l->tk==LEX_DIVEQUAL ||
			l->tk==LEX_MODEQUAL ||
			l->tk==LEX_MINUSEQUAL) {
		LEX_TYPES op = l->tk;
		lex_chkread(l, l->tk);
		base(l);
		// sort out initialiser
		if (op == '=')  {
	//		bytecode->gen(INSTR_ASIGN);
		}
		else if(op == LEX_PLUSEQUAL) {
		//	bytecode->gen(INSTR_PLUSEQ);
		}
		else if(op == LEX_MINUSEQUAL) {
		//	bytecode->gen(INSTR_MINUSEQ);
		}
		else if(op == LEX_MULTIEQUAL) {
		//	bytecode->gen(INSTR_MULTIEQ);
		}
		else if(op == LEX_DIVEQUAL) {
		//	bytecode->gen(INSTR_DIVEQ);
		}
		else if(op == LEX_MODEQUAL) {
		//	bytecode->gen(INSTR_MODEQ);
		}
	}
	return true;
}

bool block(lex_t* l) {
	lex_chkread(l, '{');

	while (l->tk && l->tk!='}'){
		if(!statement(l))
			return false;
	}

	lex_chkread(l, '}');
	return true;
}

bool statement(lex_t* l) {
	if (l->tk=='{') {
		/* A block of code */
		if(!block(l))
			return false;
	}
	else if (l->tk==LEX_ID    ||
			l->tk==LEX_INT   ||
			l->tk==LEX_FLOAT ||
			l->tk==LEX_STR   ||
			l->tk==LEX_PLUSPLUS   ||
			l->tk==LEX_MINUSMINUS ||
			l->tk=='-'    ) {
		/* Execute a simple statement that only contains basic arithmetic... */
		if(!base(l))
			return false;
		lex_chkread(l, ';');
	}
	else if (l->tk==LEX_R_VAR || l->tk == LEX_R_CONST) {
		bool beConst;
		if(l->tk == LEX_R_VAR) {
			lex_chkread(l, LEX_R_VAR);
			beConst = false;
		}
		else {
			lex_chkread(l, LEX_R_CONST);
			beConst = true;
		}

		while (l->tk != ';') {
			const char* vname = l->tkStr.cstr;
			lex_chkread(l, LEX_ID);
			// now do stuff defined with dots
			/*while (l->tk == '.') {
				lex_chkread(l, '.');
				vname = vname + "." + l->tkStr;
				lex_chkread(l, LEX_ID);
			}*/
			//bytecode->gen(beConst ? INSTR_CONST : INSTR_VAR, vname);
			// sort out initialiser
			if (l->tk == '=') {
				lex_chkread(l, '=');
				//bytecode->gen(INSTR_LOAD, vname);
				base(l);
				//bytecode->gen(INSTR_ASIGN);
				//bytecode->gen(INSTR_POP);
			}

			if (l->tk != ';')
				lex_chkread(l, ',');
		}      
		lex_chkread(l, ';');
	}

	return true;
}


bool js_exec(const char* input) {
	lex_t lex;
	lex_init(&lex, input);

	while (lex.tk) {
		if(!statement(&lex))
			break;
	}

	lex_release(&lex);
	return true;
}

#endif
