/**
very tiny js engine in single file.
*/

#ifndef MARIO_JS
#define MARIO_JS

#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#define MARIO_DUMP 

/*typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
*/

#ifndef bool
typedef int bool;
#endif

#ifndef true
#define true  1
#define false  0
#endif

#ifndef null
#define null NULL
#endif


/** memory functions.-----------------------------*/
#ifndef PRE_ALLOC
#include <stdlib.h>
	#define _malloc malloc
	#define _realloc realloc
	#define _free free
#else
/*TODO*/
#endif

typedef void (*free_func_t)(void* p);

/** array functions.-----------------------------*/

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

void* array_remove(m_array_t* array, uint32_t index) { //remove out but not free
	if(index >= array->size)
		return NULL;

	void *p = array->items[index];
	uint32_t i;
	for(i=index; i<(array->size-1); i++) {
		array->items[i] = array->items[i+1];	
	}

	array->size--;
	array->items[array->size] = NULL;
	return p;
}

void array_del(m_array_t* array, uint32_t index, free_func_t fr) { // remove out and free.
	void* p = array_remove(array, index);
	if(p != NULL) {
		if(fr != NULL)
			fr(p);
		else
			_free(p);
	}
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

/** str functions.-----------------------------*/

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

void str_inits(str_t* str, const char* s) {
	str_init(str);
	str_cpy(str, s);
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

const char* str_from_int(int i) {
	static char s[16];
	snprintf(s, 16, "%d", i);
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
		((ch>='A') && (ch<='Z')))
		return true;
	return false;
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


/** JS bytecode.-----------------------------*/

typedef uint16_t OprCode;
typedef uint32_t PC;

#define ILLEGAL_PC 0x0FFFFFFF
#define INS(ins, off) (((((int32_t)ins) << 16) & 0xFFFF0000) | ((off) & 0x0000FFFF))

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


const char* instr_str(OprCode ins) {
	switch(ins) {
		case  INSTR_NIL					: return "nil";
		case  INSTR_END					: return "end";
		case  INSTR_OBJ					: return "obj";
		case  INSTR_OBJ_END			: return "obje";
		case  INSTR_MEMBER			: return "member";
		case  INSTR_MEMBERN			: return "membern";
		case  INSTR_POP					: return "pop";
		case  INSTR_VAR					: return "var";
		case  INSTR_CONST				: return "const";
		case  INSTR_INT					: return "int";
		case  INSTR_FLOAT				: return "float";
		case  INSTR_STR					: return "str";
		case  INSTR_ARRAY_AT		: return "arrat";
		case  INSTR_ARRAY				: return "arr";
		case  INSTR_ARRAY_END		: return "arre";
		case  INSTR_LOAD				: return "load";
		case  INSTR_STORE				: return "store";
		case  INSTR_JMP					: return "jmp";
		case  INSTR_NJMP				: return "njmp";
		case  INSTR_JMPB				: return "jmpb";
		case  INSTR_NJMPB				: return "njmpb";
		case  INSTR_FUNC				: return "func";
		case  INSTR_FUNC_GET		: return "funcget";
		case  INSTR_FUNC_SET		: return "funcset";
		case  INSTR_CLASS				: return "class";
		case  INSTR_CLASS_END		: return "classe";
		case  INSTR_EXTENDS			: return "extends";
		case  INSTR_CALL				: return "call";
		case  INSTR_CALLO				: return "callo";
		case  INSTR_NOT					: return "not";
		case  INSTR_MULTI				: return "multi";
		case  INSTR_DIV					: return "div";
		case  INSTR_MOD					: return "mod";
		case  INSTR_PLUS				: return "plus";
		case  INSTR_MINUS				: return "minus";
		case  INSTR_NEG					: return "neg";
		case  INSTR_PPLUS				: return "pplus";
		case  INSTR_MMINUS			: return "mminus";
		case  INSTR_PPLUS_PRE		: return "pplusp";
		case  INSTR_MMINUS_PRE	: return "mminusp";
		case  INSTR_LSHIFT			: return "lshift";
		case  INSTR_RSHIFT			: return "rshift";
		case  INSTR_URSHIFT			: return "urshift";
		case  INSTR_EQ					: return "eq";
		case  INSTR_NEQ					: return "neq";
		case  INSTR_LEQ					: return "leq";
		case  INSTR_GEQ					: return "geq";
		case  INSTR_GRT					: return "grt";
		case  INSTR_LES					: return "les";
		case  INSTR_PLUSEQ			: return "pluseq";
		case  INSTR_MINUSEQ			: return "minuseq";
		case  INSTR_MULTIEQ			: return "multieq";
		case  INSTR_DIVEQ				: return "diveq";
		case  INSTR_MODEQ				: return "modeq";
		case  INSTR_AAND				: return "aand";
		case  INSTR_OOR					: return "oor";
		case  INSTR_OR					: return "or";
		case  INSTR_XOR					: return "xor";
		case  INSTR_AND					: return "and";
		case  INSTR_ASIGN				: return "asign";
		case  INSTR_BREAK				: return "break";
		case  INSTR_CONTINUE		: return "continue";
		case  INSTR_RETURN			: return "return";
		case  INSTR_RETURNV			: return "returnv";
		case  INSTR_TRUE				: return "true";
		case  INSTR_FALSE				: return "false";
		case  INSTR_NULL				: return "null";
		case  INSTR_UNDEF				: return "undef";
		case  INSTR_NEW					: return "new";
		case  INSTR_GET					: return "get";
		case  INSTR_BLOCK				: return "block";
		case  INSTR_BLOCK_END		: return "blocke";
		case  INSTR_THROW				: return "throw";
		case  INSTR_MOV_EXCP		: return "movexcp";
		default									: return "";
	}
}

#define BC_BUF_SIZE  3232

typedef struct st_bytecode {
	PC cindex;
	m_array_t strTable;
	PC *codeBuf;
	uint32_t bufSize;
} bytecode_t;

void bc_init(bytecode_t* bc) {
	bc->cindex = 0;
	bc->codeBuf = NULL;
	bc->bufSize = 0;
	array_init(&bc->strTable);
}

void bc_release(bytecode_t* bc) {
	array_clean(&bc->strTable, NULL);
	if(bc->codeBuf != NULL)
		_free(bc->codeBuf);
}

void bc_add(bytecode_t* bc, PC ins) {
	if(bc->cindex >= bc->bufSize) {
		bc->bufSize = bc->cindex + BC_BUF_SIZE;
		PC *newBuf = (PC*)_malloc(bc->bufSize*sizeof(PC));

		if(bc->cindex > 0 && bc->codeBuf != NULL) {
			memcpy(newBuf, bc->codeBuf, bc->cindex*sizeof(PC));
			_free(bc->codeBuf);
		}
		bc->codeBuf = newBuf;
	}

	bc->codeBuf[bc->cindex] = ins;
	bc->cindex++;
}
	
PC bc_reserve(bytecode_t* bc) {
	bc_add(bc, INS(INSTR_NIL, 0xFFFF));
  return bc->cindex-1;
}

const char* bc_getstr(bytecode_t* bc, int i) {
	if(i<0 || i == 0xFFFF ||  i>=bc->strTable.size)
		return "";
	return (const char*)bc->strTable.items[i];
}	

uint16_t bc_getstrindex(bytecode_t* bc, const char* str) {
	uint16_t sz = bc->strTable.size;
	uint16_t i;
	if(str == NULL || str[0] == 0)
		return 0xFFFF;

	for(i=0; i<sz; ++i) {
		char* s = (char*)bc->strTable.items[i];
		if(s != NULL && strcmp(s, str) == 0)
			return i;
	}
	char* p = (char*)_malloc(strlen(str) + 1);
	strcpy(p, str);
	array_add(&bc->strTable, p);
	return sz;
}	

PC bc_bytecode(bytecode_t* bc, OprCode instr, const char* str) {
	OprCode r = instr;
	OprCode i = 0xFFFF;

	if(str != NULL && str[0] != 0)
		i = bc_getstrindex(bc, str);

	return INS(r, i);
}
		
PC bc_gen_str(bytecode_t* bc, OprCode instr, const char* str) {
	int i = 0;
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
		bc_add(bc, i);
	}
	else if(instr == INSTR_FLOAT) {
		memcpy(&i, &f, sizeof(PC));
		bc_add(bc, i);
	}
	return bc->cindex;
}

PC bc_gen(bytecode_t* bc, OprCode instr) {
	return bc_gen_str(bc, instr, "");
}

PC bc_gen_int(bytecode_t* bc, OprCode instr, int i) {
	PC ins = bc_bytecode(bc, instr, "");
	bc_add(bc, ins);
	bc_add(bc, i);
	return bc->cindex;
}

void bc_set_instr(bytecode_t* bc, PC anchor, OprCode op, PC target) {
	if(target == ILLEGAL_PC)
		target = bc->cindex;

	int offset = target > anchor ? (target-anchor) : (anchor-target);
	PC ins = INS(op, offset);
	bc->codeBuf[anchor] = ins;
}

void bc_add_instr(bytecode_t* bc, PC anchor, OprCode op, PC target) {
	if(target == ILLEGAL_PC)
		target = bc->cindex;

	int offset = target > anchor ? (target-anchor) : (anchor-target);
	PC ins = INS(op, offset);
	bc_add(bc, ins);
} 

typedef void (*dump_func_t)(const char*);

#ifdef MARIO_DUMP

PC bc_get_instr_str(bytecode_t* bc, PC i, str_t* ret) {
	PC ins = bc->codeBuf[i];
	OprCode instr = (ins >> 16) & 0xFFFF;
	OprCode strIndex = ins & 0xFFFF;

	char s[32];
	str_reset(ret);

	if(strIndex == 0xFFFF) {
		sprintf(s, "  |%04d 0x%08X ; %s", i, ins, instr_str(instr));	
		str_append(ret, s);
	}
	else {
		if(instr == INSTR_JMP || 
				instr == INSTR_NJMP || 
				instr == INSTR_NJMPB ||
				instr == INSTR_JMPB) {
			sprintf(s, "  |%04d 0x%08X ; %s %d", i, ins, instr_str(instr), strIndex);	
			str_append(ret, s);
		}
		else if(instr == INSTR_STR) {
			sprintf(s, "  |%04d 0x%08X ; %s \"", i, ins, instr_str(instr));	
			str_append(ret, s);
			str_append(ret, bc_getstr(bc, strIndex));
		}
		else {
			sprintf(s, "  |%04d 0x%08X ; %s ", i, ins, instr_str(instr));	
			str_append(ret, s);
			str_append(ret, bc_getstr(bc, strIndex));
		}
	}
	
	if(instr == INSTR_INT) {
		ins = bc->codeBuf[i+1];
		sprintf(s, "\n  |%04d 0x%08X ; (%d)", i+1, ins, ins);	
		str_append(ret, s);
		i++;
	}
	else if(instr == INSTR_FLOAT) {
		ins = bc->codeBuf[i+1];
		float f;
		memcpy(&f, &ins, sizeof(PC));
		sprintf(s, "\n  |%04d 0x%08X ; (%f)", i+1, ins, f);	
		str_append(ret, s);
		i++;
	}	
	return i;
}

void bc_dump(bytecode_t* bc, dump_func_t dump) {
	PC i;
	PC ins = 0;
	char index[8];
	PC sz = bc->strTable.size;

	dump("-------string table--------------------\n");
	for(i=0; i<sz; ++i) {
		sprintf(index, "%04X: ", i);
		dump(index);
		dump((const char*)bc->strTable.items[i]);
		dump("\n");
	}
	dump("---------------------------------------\n");

	int line = -1;
	str_t s;
	str_init(&s);

	i = 0;
	while(i < bc->cindex) {
		i = bc_get_instr_str(bc, i, &s);
		dump(s.cstr);
		dump("\n");
		i++;
	}
	str_release(&s);
}

#endif

/** Compiler -----------------------------*/

bool statement(lex_t*, bytecode_t*, bool);
bool factor(lex_t*, bytecode_t*);
bool base(lex_t*, bytecode_t*);

int callFunc(lex_t* l, bytecode_t* bc) {
	lex_chkread(l, '(');
	int argNum = 0;
	while(true) {
		PC pc1 = bc->cindex;
		if(!base(l, bc))
			return -1;
		PC pc2 = bc->cindex;
		if(pc2 > pc1) //not empty, means valid arguemnt.
			argNum++;

		if (l->tk!=')')
			lex_chkread(l, ',');	
		else
			break;
	}
	lex_chkread(l, ')');
	return argNum;
}
		
bool block(lex_t* l, bytecode_t* bc) {
	lex_chkread(l, '{');

	while (l->tk && l->tk!='}'){
		if(!statement(l, bc, true))
			return false;
	}

	lex_chkread(l, '}');
	return true;
}

bool defFunc(lex_t* l, bytecode_t* bc, str_t* name) {
	/* we can have functions without names */
	if (l->tk == LEX_ID) {
		str_cpy(name, l->tkStr.cstr);
		lex_chkread(l, LEX_ID);
	}
	
	if(l->tk == LEX_ID) { //class get/set token
		if(strcmp(name->cstr, "get") == 0) {
			str_cpy(name, l->tkStr.cstr);
			lex_chkread(l, LEX_ID);
			bc_gen(bc, INSTR_FUNC_GET);
		}
		if(strcmp(name->cstr, "set") == 0) {
			str_cpy(name, l->tkStr.cstr);
			lex_chkread(l, LEX_ID);
			bc_gen(bc, INSTR_FUNC_SET);
		}
	}
	else {
		bc_gen(bc, INSTR_FUNC);
	}
	//do arguments
	lex_chkread(l, '(');
	while (l->tk!=')') {
		bc_gen_str(bc, INSTR_VAR, l->tkStr.cstr);
		lex_chkread(l, LEX_ID);
		if (l->tk!=')') 
			lex_chkread(l, ',');
	}
	lex_chkread(l, ')');
	PC pc = bc_reserve(bc);
	block(l, bc);
	
	OprCode op = bc->codeBuf[bc->cindex - 1] >> 16;

	if(op != INSTR_RETURN && op != INSTR_RETURNV)
		bc_gen(bc, INSTR_RETURN);
	bc_set_instr(bc, pc, INSTR_JMP, ILLEGAL_PC);
	return true;
}

bool factor(lex_t* l, bytecode_t* bc) {
	if (l->tk=='(') {
		lex_chkread(l, '(');
		base(l, bc);
		lex_chkread(l, ')');
	}
	else if (l->tk==LEX_R_TRUE) {
		lex_chkread(l, LEX_R_TRUE);
		bc_gen(bc, INSTR_TRUE);
	}
	else if (l->tk==LEX_R_FALSE) {
		lex_chkread(l, LEX_R_FALSE);
		bc_gen(bc, INSTR_FALSE);
	}
	else if (l->tk==LEX_R_NULL) {
		lex_chkread(l, LEX_R_NULL);
		bc_gen(bc, INSTR_NULL);
	}
	else if (l->tk==LEX_R_UNDEFINED) {
		lex_chkread(l, LEX_R_UNDEFINED);
		bc_gen(bc, INSTR_UNDEF);
	}
	else if (l->tk==LEX_INT) {
		bc_gen_str(bc, INSTR_INT, l->tkStr.cstr);
		lex_chkread(l, LEX_INT);
	}
	else if (l->tk==LEX_FLOAT) {
		bc_gen_str(bc, INSTR_FLOAT, l->tkStr.cstr);
		lex_chkread(l, LEX_FLOAT);
	}
	else if (l->tk==LEX_STR) {
		bc_gen_str(bc, INSTR_STR, l->tkStr.cstr);
		lex_chkread(l, LEX_STR);
	}
	else if(l->tk==LEX_R_FUNCTION) {
		lex_chkread(l, LEX_R_FUNCTION);
		str_t fname;
		str_init(&fname);
		defFunc(l, bc, &fname);
		str_release(&fname);
	}
	else if(l->tk==LEX_R_CLASS) {
		//defClass();
	}
	else if (l->tk==LEX_R_NEW) {
		// new -> create a new object
		lex_chkread(l, LEX_R_NEW);
		str_t className;
		str_init(&className);
		str_cpy(&className, l->tkStr.cstr);

		lex_chkread(l, LEX_ID);
		if (l->tk == '(') {
			//lex_chkread(l, '(');
			int argNum = callFunc(l, bc);
			//lex_chkread(l, ')');
			if(argNum > 0) {
				str_append(&className, "$");
				str_append(&className, str_from_int(argNum));
			}
			bc_gen_str(bc, INSTR_NEW, className.cstr);
		}
		str_release(&className);
	}

	if (l->tk=='{') {
		// JSON-style object definition
		lex_chkread(l, '{');
		bc_gen(bc, INSTR_OBJ);
		while (l->tk != '}') {
			str_t id;
			str_inits(&id, l->tkStr.cstr);
			// we only allow strings or IDs on the left hand side of an initialisation
			if (l->tk==LEX_STR) 
				lex_chkread(l, LEX_STR);
			else 
				lex_chkread(l, LEX_ID);
			lex_chkread(l, ':');
			base(l, bc);
			bc_gen_str(bc, INSTR_MEMBERN, id.cstr);
			// no need to clean here, as it will definitely be used
			if (l->tk != '}') 
				lex_chkread(l, ',');
			str_release(&id);
		}
		bc_gen(bc, INSTR_OBJ_END);
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
				str_inits(&s, (const char*)names.items[sz]);
					
				if(sz == 0 && load) {
					bc_gen_str(bc, INSTR_LOAD, "this");	
					int argNum = callFunc(l, bc);
					if(argNum > 0) {
						str_append(&s, "$");
						str_append(&s, str_from_int(argNum));
					}
					bc_gen_str(bc, INSTR_CALL, s.cstr);	
				}
				else {
					int i;
					for(i=0; i<sz; i++) {
						bc_gen_str(bc, load ? INSTR_LOAD:INSTR_GET, (const char*)names.items[i]);	
						load = false;
					}
					int argNum = callFunc(l, bc);
					if(argNum > 0) {
						str_append(&s, "$");
						str_append(&s, str_from_int(argNum));
					}
					bc_gen_str(bc, INSTR_CALLO, s.cstr);	
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
					bc_gen_str(bc, load ? INSTR_LOAD:INSTR_GET, (const char*)names.items[i]);	
					load = false;
				}

				lex_chkread(l, '[');
				base(l, bc);
				lex_chkread(l, ']');
				bc_gen(bc, INSTR_ARRAY_AT);
			} 
		}
		if(name.len > 0) {
			int i, sz;
			str_split(name.cstr, '.', &names);
			str_reset(&name);
			sz = names.size;
			for(i=0; i<sz; i++) {
				bc_gen_str(bc, load ? INSTR_LOAD:INSTR_GET, (const char*)names.items[i]);	
				load = false;
			}
		}
		str_release(&name);
	}
	else if (l->tk=='[') {
		// JSON-style array 
		lex_chkread(l, '[');
		bc_gen(bc, INSTR_ARRAY);
		while (l->tk != ']') {
			base(l, bc);
			bc_gen(bc, INSTR_MEMBER);
			if (l->tk != ']') 
				lex_chkread(l, ',');
		}
		lex_chkread(l, ']');
		bc_gen(bc, INSTR_ARRAY_END);
	}

	return true;
}

bool unary(lex_t* l, bytecode_t* bc) {
	OprCode instr = INSTR_END;
	if (l->tk == '!') {
		lex_chkread(l, '!');
		instr = INSTR_NOT;
	} else if(l->tk == LEX_R_TYPEOF) {
		lex_chkread(l, LEX_R_TYPEOF);
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
		lex_chkread(l, l->tk);
		unary(l, bc);

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
		lex_chkread(l, '-');
	}
	else if(l->tk==LEX_PLUSPLUS) {
		lex_chkread(l, LEX_PLUSPLUS);
	}
	else if(l->tk==LEX_MINUSMINUS) {
		lex_chkread(l, LEX_MINUSMINUS);
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
		lex_chkread(l, l->tk);
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
		lex_chkread(l, op);
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
		lex_chkread(l, l->tk);
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
		lex_chkread(l, l->tk);
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
		lex_chkread(l, '?');
		base(l, bc); //first choice
		PC pc2 = bc_reserve(bc); //keep for jump
		lex_chkread(l, ':');
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
		lex_chkread(l, l->tk);
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

bool statement(lex_t* l, bytecode_t* bc, bool pop) {
	if (l->tk=='{') {
		/* A block of code */
		if(!block(l, bc))
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
		lex_chkread(l, ';');
	}
	else if (l->tk==LEX_R_VAR || l->tk == LEX_R_CONST) {
		pop = false;
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
			str_t vname;
			str_inits(&vname, l->tkStr.cstr);
			lex_chkread(l, LEX_ID);
			// now do stuff defined with dots
			/*while (l->tk == '.') {
				lex_chkread(l, '.');
				vname = vname + "." + l->tkStr;
				lex_chkread(l, LEX_ID);
			}*/
			bc_gen_str(bc, beConst ? INSTR_CONST : INSTR_VAR, vname.cstr);
			// sort out initialiser
			if (l->tk == '=') {
				lex_chkread(l, '=');
				bc_gen_str(bc, INSTR_LOAD, vname.cstr);
				base(l, bc);
				bc_gen(bc, INSTR_ASIGN);
				bc_gen(bc, INSTR_POP);
			}
			if (l->tk != ';')
				lex_chkread(l, ',');
			str_release(&vname);
		}
		lex_chkread(l, ';');
	}
	else if(l->tk == LEX_R_FUNCTION) {
		lex_chkread(l, LEX_R_FUNCTION);
		str_t fname;
		str_init(&fname);
		defFunc(l, bc, &fname);
		bc_gen_str(bc, INSTR_MEMBERN, fname.cstr);
		str_release(&fname);
	}
	else if (l->tk == LEX_R_RETURN) {
		lex_chkread(l, LEX_R_RETURN);
		pop = false;
		if (l->tk != ';') {
			base(l, bc);
			bc_gen(bc, INSTR_RETURNV);
		}
		else {
			bc_gen(bc, INSTR_RETURN);
		}
		lex_chkread(l, ';');
	} 
	else if (l->tk == LEX_R_IF) {
		lex_chkread(l, LEX_R_IF);
		lex_chkread(l, '(');
		base(l, bc); //condition
		lex_chkread(l, ')');
		PC pc = bc_reserve(bc);
		statement(l, bc, true);

		if (l->tk == LEX_R_ELSE) {
			lex_chkread(l, LEX_R_ELSE);
			PC pc2 = bc_reserve(bc);
			bc_set_instr(bc, pc, INSTR_NJMP, ILLEGAL_PC);
			statement(l, bc, true);
			bc_set_instr(bc, pc2, INSTR_JMP, ILLEGAL_PC);
		}
		else {
			bc_set_instr(bc, pc, INSTR_NJMP, ILLEGAL_PC);
		}
		pop = false;
	}

	if(pop)
		bc_gen(bc, INSTR_POP);
	return true;
}

bool compile(bytecode_t *bc, const char* input, dump_func_t dump) {
	lex_t lex;
	lex_init(&lex, input);

	while(lex.tk) {
		if(!statement(&lex, bc, true)) {
			lex_release(&lex);
			return false;
		}
		bc_gen(bc, INSTR_END);
	}

	lex_release(&lex);
#ifdef MARIO_DUMP
	if(dump != NULL)
		bc_dump(bc, dump);
#endif
	return true;
}

/** vm var-----------------------------*/
#define V_UNDEF  0
#define V_INT    1
#define V_FLOAT  2
#define V_STRING 3
#define V_POINT  4
#define V_FUNC   5
#define V_NFUNC  6
#define V_OBJECT 7
#define V_CLASS  8
#define V_BYTES  9
#define V_ARRAY  10

//script var
typedef struct st_var {
	int32_t magic; //0 for var
	int32_t refs:24;
	int32_t type:8;

	void* value;

	m_array_t children;
} var_t;

//script node for var member children
typedef struct st_node {
	int32_t magic; //1 for node
	char* name;
	bool beConst;
	var_t* var;
} node_t;

var_t* var_new();
var_t* var_ref(var_t*);
void var_unref(var_t*, bool);

node_t* node_new(const char* name) {
	node_t* node = (node_t*)_malloc(sizeof(node_t));
	node->magic = 1;
	node->name = (char*)_malloc(strlen(name)+1);
	node->beConst = false;
	strcpy(node->name, name);
	node->var = var_ref(var_new());	
	return node;
}

void node_free(void* p) {
	node_t* node = (node_t*)p;
	if(node == NULL)
		return;

	_free(node->name);
	var_unref(node->var, true);
	_free(node);
}

var_t* node_replace(node_t* node, var_t* v) {
	var_t* old = node->var;
	node->var = var_ref(v);
	var_unref(old, true);
	return node->var;
}

void var_removeAll(var_t* var) {
	/*free children*/
	array_clean(&var->children, node_free);
}

node_t* var_find(var_t* var, const char*name) {
	int i;
	for(i=0; i<var->children.size; i++) {
		node_t* node = (node_t*)var->children.items[i];
		if(node != NULL) {
			if(strcmp(node->name, name) == 0)
				return node;
		}
	}
	return NULL;
}

node_t* var_add(var_t* var, const char* name) {
	node_t* node = node_new(name);
	if(node != NULL) {
		array_add(&var->children, node);
	}
	return node;
}

void var_free(void* p) {
	var_t* var = (var_t*)p;
	if(var == NULL || var->refs > 0)
		return;

	/*free children*/
	var_removeAll(var);	

	/*free value*/
	if(var->value != NULL)
		_free(var->value);
	
	_free(var);
}

void var_unref(var_t* var, bool del) {
	if(var != NULL) {
		var->refs--;
		if(var->refs <= 0 && del) {
			var_free(var);
		}
	}
}

var_t* var_ref(var_t* var) {
	if(var != NULL)
		var->refs++;
	return var;
}

var_t* var_new() {
	var_t* var = (var_t*)_malloc(sizeof(var_t));
	var->magic = 0;
	var->refs = 0;
	var->type = V_UNDEF;
	
	var->value = NULL;
	array_init(&var->children);
	return var;
}

var_t* var_new_object() {
	var_t* var = var_new();
	var->type = V_OBJECT;
	return var;
}

var_t* var_new_int(int i) {
	var_t* var = var_new();
	var->type = V_INT;
	var->value = _malloc(sizeof(int));
	memcpy(var->value, &i, sizeof(int));
	return var;
}

var_t* var_new_float(float i) {
	var_t* var = var_new();
	var->type = V_FLOAT;
	var->value = _malloc(sizeof(float));
	memcpy(var->value, &i, sizeof(float));
	return var;
}

var_t* var_new_str(const char* s) {
	var_t* var = var_new();
	var->type = V_STRING;
	var->value = _malloc(strlen(s) + 1);
	strcpy((char*)var->value, s);
	return var;
}

/** Interpreter-----------------------------*/

typedef struct st_vm {
	bytecode_t bc;
	m_array_t scopes;
	m_array_t stack;

	var_t* root;
} vm_t;

var_t* vm_push_var(vm_t* vm, var_t* var) {
	var_ref(var);
	array_add(&vm->stack, var);
	return var;
}

node_t* vm_push_node(vm_t* vm, node_t* node) {
	if(node == NULL)
		return NULL;

	var_ref(node->var);
	array_add(&vm->stack, node);
	return node;
}

void vm_pop(vm_t* vm) {
	if(vm->stack.size <= 0)
		return;

	int32_t magic = *(int32_t*)vm->stack.items[vm->stack.size-1];
	if(magic == 0) {//var
		var_t* var = (var_t*)array_remove(&vm->stack, vm->stack.size-1);
		var_unref(var, true);
	}
	else { //node
		node_t* node = (node_t*)array_remove(&vm->stack, vm->stack.size-1);
		if(node != NULL)
			var_unref(node->var, true);
	}
}

node_t* vm_pop2node(vm_t* vm) {
	int index = vm->stack.size-1;
	if(index < 0)
		return NULL;

	int32_t magic = *(int32_t*)vm->stack.items[index];
	if(magic != 1) //not node!
		return NULL;

	return (node_t*)array_remove(&vm->stack, index);
}

var_t* vm_pop2var(vm_t* vm) {
	int index = vm->stack.size-1;
	if(index < 0)
		return NULL;

	int32_t magic = *(int32_t*)vm->stack.items[index];
	if(magic != 0) //not var!
		return NULL;

	return (var_t*)array_remove(&vm->stack, index);
}

//scope of vm runing
typedef struct st_scope {
	var_t* var;
	PC pc; //stack pc
} scope_t;

scope_t* scope_new(var_t* var, PC pc) {
	scope_t* sc = (scope_t*)_malloc(sizeof(scope_t));
	sc->var = var_ref(var);
	sc->pc = pc;
	return sc;
}

void scope_free(void* p) {
	scope_t* sc = (scope_t*)p;
	if(sc == NULL)
		return;
	var_unref(sc->var, true);
	_free(sc);
}

scope_t* vm_get_scope(vm_t* vm) {
	int i = ((int)vm->scopes.size) - 1;
	return (i < 0 ? NULL : (scope_t*)vm->scopes.items[i]);
}

void vm_push_scope(vm_t* vm, scope_t* sc) {
	array_add(&vm->scopes, sc);	
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
	array_del(&vm->scopes, vm->scopes.size-1, scope_free);
	return pc;
}

void vm_init(vm_t* vm) {
	bc_init(&vm->bc);
	array_init(&vm->stack);	
	array_init(&vm->scopes);	

	vm->root = var_ref(var_new_object());
}

void vm_close(vm_t* vm) {
	var_unref(vm->root, true);

	array_clean(&vm->scopes, NULL);	
	array_clean(&vm->stack, var_free);	
	bc_release(&vm->bc);
}	

bool vm_load(vm_t* vm, const char* s, dump_func_t dump) {
	return compile(&vm->bc, s, dump);
}

node_t* vm_find(vm_t* vm, const char* name) {
	scope_t* sc = vm_get_scope(vm);
	if(sc == NULL || sc->var == NULL)
		return NULL;
	return var_find(sc->var, name);	
}

node_t* vm_load_node(vm_t* vm, const char* name, bool create) {
	scope_t* sc = vm_get_scope(vm);
	var_t* var = vm->root;
	if(sc != NULL && sc->var != NULL)
		var = sc->var;

	node_t* n =  var_find(var, name);	
	if(n != NULL)
		return n;

	if(create)
		n =var_add(var, name);	
	else
		n = NULL;

	return n;
}

void vm_run_code(vm_t* vm) {
	int32_t scDeep = vm->scopes.size;
	bool needPopSC = false;
	PC pc = 0;
	PC codeSize = vm->bc.cindex;
	PC* code = vm->bc.codeBuf;

	while(pc < codeSize) {
		PC ins = code[pc++];
		OprCode instr = ins >> 16;
		OprCode offset = ins & 0x0000FFFF;
		str_t str;

		if(instr == INSTR_END)
			break;
		
		switch(instr) {
			case INSTR_NIL: {	break; }
			case INSTR_BLOCK: 
			{
				scope_t* bl = vm_get_scope(vm);
				scope_t* sc;
				if(bl != NULL) 
					sc = scope_new(bl->var, bl->pc);
				else
					sc = scope_new(NULL, 0);
				vm_push_scope(vm, sc);
				break;
			}
			case INSTR_BLOCK_END: 
			{
				vm_pop_scope(vm);
				break;
			}
			case INSTR_TRUE: 
			{
				var_t* v = var_new_int(1);	
				vm_push_var(vm, v);
				break;
			}
			case INSTR_FALSE: 
			{
				var_t* v = var_new_int(1);	
				vm_push_var(vm, v);
				break;
			}
			case INSTR_UNDEF: 
			{
				var_t* v = var_new();	
				vm_push_var(vm, v);
				break;
			}
			case INSTR_POP: 
			{
				vm_pop(vm);
				break;
			}
			case INSTR_JMP: 
			{
				pc = pc + offset - 1;
				break;
			}
			case INSTR_JMPB: 
			{
				pc = pc - offset - 1;
				break;
			}
			/*
			case INSTR_NJMP: {
												 StackItem* i = pop2();
												 if(i != NULL) {
													 BCVar* v = VAR(i);
													 if(v->type == BCVar::UNDEF || v->getInt() == 0)
														 pc = pc + offset - 1;
													 v->unref();
												 }
												 break;
											 }
			case INSTR_NEG: {
												StackItem* i = pop2();
												if(i != NULL) {
													BCVar* v = VAR(i);
													if(v->isInt()) {
														int n = v->getInt();
														v->setInt(-n);
													}
													else if(v->isFloat()) {
														float n = v->getFloat();
														v->setFloat(-n);
													}
													push(v);
												}
												break;
											}
			case INSTR_NOT: {
												StackItem* i = pop2();
												if(i != NULL) {
													BCVar* v = VAR(i);
													int c = 0;
													if(v->type == BCVar::UNDEF || v->getInt() == 0)
														c = 1;
													v->unref();
													v = new BCVar(c);
													push(v->ref());	
												}
												break;
											}
			case INSTR_EQ: 
			case INSTR_NEQ: 
			case INSTR_TEQ:
			case INSTR_NTEQ:
			case INSTR_LES: 
			case INSTR_GRT: 
			case INSTR_LEQ: 
			case INSTR_GEQ: {
												StackItem* i2 = pop2();
												StackItem* i1 = pop2();
												if(i1 != NULL && i2 != NULL) {
													BCVar* v1 = VAR(i1);
													BCVar* v2 = VAR(i2);
													compare(instr, v1, v2);

													v1->unref();
													v2->unref();
												}
												break;
											}
			case INSTR_AAND: 
			case INSTR_OOR: {
												StackItem* i2 = pop2();
												StackItem* i1 = pop2();
												if(i1 != NULL && i2 != NULL) {
													BCVar* v1 = VAR(i1);
													BCVar* v2 = VAR(i2);

													int r = 0;
													if(instr == INSTR_AAND)
														r = (v1->getInt() != 0) && (v2->getInt() != 0);
													else
														r = (v1->getInt() != 0) || (v2->getInt() != 0);
													BCVar* v = new BCVar(r);
													push(v->ref());

													v1->unref();
													v2->unref();
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
			case INSTR_MOD: {
												StackItem* i2 = pop2();
												StackItem* i1 = pop2();
												if(i1 != NULL && i2 != NULL) {
													BCVar* v1 = VAR(i1);
													BCVar* v2 = VAR(i2);
													mathOp(instr, v1, v2);

													v1->unref();
													v2->unref();
												}
												break;
											}
			case INSTR_MMINUS_PRE: {
															 StackItem* it = pop2();
															 if(it != NULL) {
																 BCVar* v = VAR(it);
																 int i = v->getInt() - 1;
																 v->setInt(i);
																 push(v);
															 }
															 break;
														 }
			case INSTR_MMINUS: {
													 StackItem* it = pop2();
													 if(it != NULL) {
														 BCVar* v = VAR(it);
														 int i = v->getInt();
														 v->setInt(i-1);
														 v->unref();
														 v = new BCVar(i);
														 push(v->ref());
													 }
													 break;
												 }
			case INSTR_PPLUS_PRE: {
															StackItem* it = pop2();
															if(it != NULL) {
																BCVar* v = VAR(it);
																int i = v->getInt() + 1;
																v->setInt(i);
																push(v);
															}
															break;
														}
			case INSTR_PPLUS: {
													StackItem* it = pop2();
													if(it != NULL) {
														BCVar* v = VAR(it);
														int i = v->getInt();
														v->setInt(i+1);
														v->unref();
														v = new BCVar(i);
														push(v->ref());
													}
													break;
												}
			case INSTR_RETURN:  //return without value
			case INSTR_RETURNV: { //return with value
														VMScope* sc = scope();
														if(sc != NULL) {
															if(instr == INSTR_RETURN) {//return without value, push "this" to stack
																BCVar* thisVar = sc->var->getThisVar();
																push(thisVar->ref());
															}
															else {
																StackItem* it = pop2();
																if(it != NULL)
																push(VAR(it));
															}

															pc = sc->pc;
															if(scDeep == scopes.size() && bc == NULL) { //usually means call js function.
																popScope();
																return;
															}
															popScope();
															if(scopes.size() == 0)  //scope size = 0, not return in function,means exit.
																return;
														}
														break;
													}
					*/
			case INSTR_VAR:
			case INSTR_CONST: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				node_t *node = vm_find(vm, s);
				if(node != NULL) { //find just in current scope
					//TODO
					//ERR("Warning: %s has already existed. %s\n", str.c_str(), DEBUG_LINE);
				}
				else {
					scope_t* sc = vm_get_scope(vm);
					if(sc != NULL) 
						node = var_add(sc->var, s);
					else 
						node = var_add(vm->root, s);

					if(node != NULL && instr == INSTR_CONST)
						node->beConst = true;
				}
				break;
			}
			case INSTR_INT:
			{
				var_t* v = var_new_int((int)code[pc++]);
				vm_push_var(vm, v);
				break;
			}
			case INSTR_FLOAT: 
			{
				var_t* v = var_new_float((float)code[pc++]);
				vm_push_var(vm, v);
				break;
			}
			case INSTR_STR: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				var_t* v = var_new_str(s);
				vm_push_var(vm, v);
				break;
			}
			case INSTR_ASIGN: 
			{
				var_t* v = vm_pop2var(vm);
				node_t* n = vm_pop2node(vm);
				if(v != NULL && n != NULL) {
					bool modi = (!n->beConst || n->var->type == V_UNDEF);
					var_unref(n->var, true);
					if(modi) 
						node_replace(n, v);
					else {
					//TODO
					//	ERR("Can not change a const variable: %s! %s\n", node->name.c_str(), DEBUG_LINE);
					}
					var_unref(v, true);
					vm_push_var(vm, n->var);
				}
				break;
			}
			case INSTR_LOAD: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				node_t* n = vm_load_node(vm, s, true); //load variable, create if not exist.
				vm_push_node(vm, n);
				break;
			}
			case INSTR_GET: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				var_t* v = vm_pop2var(vm);
				if(v != NULL) {
					//doGet(v, s); //TODO
					var_unref(v, true);
				}
				break;
			}

			/*
			case INSTR_ARRAY_AT: {
														 StackItem* i2 = pop2();
														 StackItem* i1 = pop2();
														 if(i1 != NULL && i1->isNode && i2 != NULL) {
															 BCNode* node = (BCNode*)i1;

															 BCVar* v = VAR(i2);
															 int at = v->getInt();
															 v->unref();

															 BCNode* n = node->var->getChildOrCreate(at);
															 if(n != NULL) {
																 n->var->ref();
																 push(n);
															 }
															 node->var->unref();
														 }
														 break;
													 }
			case INSTR_OBJ:
			case INSTR_ARRAY: {
													BCVar* obj = new BCVar();
													if(instr == INSTR_OBJ)
														obj->type = BCVar::OBJECT;
													else
														obj->type = BCVar::ARRAY;
													VMScope sc;
													sc.var = obj->ref();
													pushScope(sc);
													break;
												}
			case INSTR_ARRAY_END: 
			case INSTR_OBJ_END: {
														BCVar* obj = scope()->var;
														push(obj->ref()); //that actually means currentObj->ref() for push and unref for unasign.
														popScope();
														break;
													}
			case INSTR_MEMBER: 
			case INSTR_MEMBERN: {
														str = instr == INSTR_MEMBER ? "" : bcode->getStr(offset);
														StackItem* i = pop2();
														if(i != NULL) {
															BCVar* v = VAR(i);
															if(v->isFunction()) {
																FuncT* func = v->getFunc();
																size_t argNum = func->args.size();
																if(argNum > 0)
																	str = str + "$" + StringUtil::from((int)argNum);
															}
															scope()->var->addChild(str, v);
															v->unref();
														}
														break;
													}
			case INSTR_FUNC: 
			case INSTR_FUNC_GET: 
			case INSTR_FUNC_SET: {
														 str = bcode->getStr(offset);
														 BCVar* v = funcDef(str, (instr == INSTR_FUNC ? true:false));
														 if(v != NULL)
															 push(v->ref());
														 break;
													 }
			case INSTR_CLASS: {
													str = bcode->getStr(offset);
													BCVar* v = getOrAddClass(str);
													//read extends
													ins = code[pc];
													instr = ins >> 16;
													if(instr == INSTR_EXTENDS) {
														pc++;
														offset = ins & 0x0000FFFF;
														str = bcode->getStr(offset);
														doExtends(v, str);
													}

													VMScope sc;
													sc.var = v->ref();
													pushScope(sc);
													break;
												}
			case INSTR_CLASS_END: {
															BCVar* v = scope()->var;
															push(v->ref());
															popScope();
															break;
														}
			case INSTR_CALL: 
			case INSTR_CALLO: {
												 str = bcode->getStr(offset);
												 size_t pos = str.find("$");
												 int argsNum = 0;
												 if(pos != string::npos) {
													 string args = str.substr(pos+1);
													 argsNum = atoi(args.c_str());
												 }

												 StackItem* i = vStack[stackTop+argsNum];
												 BCVar* obj = VAR(i);
												 BCNode* func = findFunc(obj, str, true);
												 if(func != NULL) {
													 funcCall(NULL, func->var);
												 }
												 doInterrupt();
												 break;
												}
			case INSTR_NEW: {
												doNew(bcode->getStr(offset));
												break;
											}
			case INSTR_TYPEOF: {
												StackItem* i = pop2();
												BCVar* var = VAR(i);
												string tp = var ? var->getTypeString() : "null" ;
												BCVar *type = new BCVar(tp);
												push(type->ref());
												break;
											}
			case INSTR_THROW: {
				BCVar *var = reinterpret_cast<BCVar*>(pop2());
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
					ERR("uncaught exception:%s\n", exception->getString().c_str());
					return;
				}

				break;
			}
			case INSTR_MOV_EXCP: {
				BCNode *node = reinterpret_cast<BCNode*>(pop2());
				node->replace(VAR(exception));
				exception = NULL;
				break;
			}
			*/
		}
	}
	if(needPopSC)
		vm_pop_scope(vm);
}
bool vm_run(vm_t* vm) {
	vm_run_code(vm);
	return true;
}

#endif
