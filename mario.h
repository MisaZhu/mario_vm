#ifndef MARIO_JS
#define MARIO_JS

typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;

typedef int bool;
static const int true = 1;
static const int false = 0;

#include <string.h>

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
		for(int i=0; i<array->size; i++) {
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
	lex_reset(lex);
}

void lex_free(lex_t* lex) {
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

LEX_TYPES block(lex_t* l) {
	LEX_TYPES ret = LEX_EOF;
	lex_chkread(l, '{');

	while (l->tk && l->tk!='}'){
		statement(l);
	}

	lex_chkread(l, '}');
	return ret;
}

bool statement(lex_t* l) {
	if (l->tk=='{') {
		/* A block of code */
		block(l);
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
	return true;
}

#endif
