/**
very tiny js engine in single file.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "mario_js.h"

void _free_none(void* p) { (void)p; }

void (*_debug_func)(const char*) = NULL;

void _debug(const char* s) {
	if(_debug_func != NULL)
		_debug_func(s);
}


/** array functions.-----------------------------*/

#define ARRAY_BUF 16

#define array_init(array) ({ \
	(array)->items = NULL; \
	(array)->size = 0; \
	(array)->max = 0; })


//void* array_add(m_array_t* array, void* item) {
#define array_add(array, it) \
	void* _item_ = (it); \
	int new_size = (array)->size + 1; \
	if((array)->max <= new_size) { \
		new_size = (array)->size + ARRAY_BUF; /*ARRAY BUF for buffer*/ \
		(array)->items = (void**)_realloc((array)->items, new_size*sizeof(void*)); \
		(array)->max = new_size; \
	} \
	(array)->items[(array)->size] = _item_; \
	(array)->size++; \
	(array)->items[(array)->size] = NULL; \

void* array_add_buf(m_array_t* array, void* s, uint32_t sz) {
	void* item = _malloc(sz);
	if(s != NULL)
		memcpy(item, s, sz);
	array_add(array, item);
	return item;
}

void* array_get(m_array_t* array, uint32_t index) {
	if(array->items == NULL || index >= array->size)
		return NULL;
	return array->items[index];
}

/*void* array_tail(m_array_t* array) {
	if(array->items == NULL || array->size == 0)
		return NULL;
	return array->items[array->size-1];
}
*/

#define array_tail(array) (((array)->items == NULL || (array)->size == 0) ? \
	NULL : ((array)->items[(array)->size-1]))


void* array_head(m_array_t* array) {
	if(array->items == NULL || array->size == 0)
		return NULL;
	return array->items[0];
}

void* array_remove(m_array_t* array, uint32_t index) { //remove out but not free
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

void array_del(m_array_t* array, uint32_t index, free_func_t fr) { // remove out and free.
	void* p = array_remove(array, index);
	if(p != NULL) {
		if(fr != NULL)
			fr(p);
		else
			_free(p);
	}
}

void array_remove_all(m_array_t* array) { //remove all items bot not free them.
	if(array->items != NULL) {
		_free(array->items);
		array->items = NULL;
	}
	array->max = array->size = 0;
}

void array_clean(m_array_t* array, free_func_t fr) { //remove all items and free them.
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

	int new_size = len;
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
	int new_size = str->len + len;
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

void str_free(str_t* str) {
	if(str == NULL)
		return;

	if(str->cstr != NULL) {
		_free(str->cstr);
	}
	_free(str);
}

static char _s[32];
const char* str_from_int(int i) {
	snprintf(_s, 31, "%d", i);
	return _s;
}

const char* str_from_float(float i) {
	snprintf(_s, 31, "%f", i);
	return _s;
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

	int32_t dataPos;
	int32_t dataStart, dataEnd;
	char currCh, nextCh;

	LEX_TYPES tk;
	str_t* tkStr;
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
	str_reset(lex->tkStr);

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
			str_add(lex->tkStr, lex->currCh);
			lex_get_nextch(lex);
		}
		lex->tk = LEX_ID;
		if (strcmp(lex->tkStr->cstr, "if") == 0)        lex->tk = LEX_R_IF;
		else if (strcmp(lex->tkStr->cstr, "else") == 0)      lex->tk = LEX_R_ELSE;
		else if (strcmp(lex->tkStr->cstr, "do") == 0)        lex->tk = LEX_R_DO;
		else if (strcmp(lex->tkStr->cstr, "while") == 0)    lex->tk = LEX_R_WHILE;
		else if (strcmp(lex->tkStr->cstr, "import") == 0)  lex->tk = LEX_R_INCLUDE;
		else if (strcmp(lex->tkStr->cstr, "for") == 0)     lex->tk = LEX_R_FOR;
		else if (strcmp(lex->tkStr->cstr, "break") == 0)    lex->tk = LEX_R_BREAK;
		else if (strcmp(lex->tkStr->cstr, "continue") == 0)  lex->tk = LEX_R_CONTINUE;
		else if (strcmp(lex->tkStr->cstr, "function") == 0)  lex->tk = LEX_R_FUNCTION;
		else if (strcmp(lex->tkStr->cstr, "class") ==0) 		 lex->tk = LEX_R_CLASS;
		else if (strcmp(lex->tkStr->cstr, "extends") == 0) 	 lex->tk = LEX_R_EXTENDS;
		else if (strcmp(lex->tkStr->cstr, "return") == 0)   lex->tk = LEX_R_RETURN;
		else if (strcmp(lex->tkStr->cstr, "var")  == 0)      lex->tk = LEX_R_VAR;
		else if (strcmp(lex->tkStr->cstr, "let")  == 0)      lex->tk = LEX_R_LET;
		else if (strcmp(lex->tkStr->cstr, "const") == 0)     lex->tk = LEX_R_CONST;
		else if (strcmp(lex->tkStr->cstr, "true") == 0)      lex->tk = LEX_R_TRUE;
		else if (strcmp(lex->tkStr->cstr, "false") == 0)     lex->tk = LEX_R_FALSE;
		else if (strcmp(lex->tkStr->cstr, "null") == 0)      lex->tk = LEX_R_NULL;
		else if (strcmp(lex->tkStr->cstr, "undefined") == 0) lex->tk = LEX_R_UNDEFINED;
		else if (strcmp(lex->tkStr->cstr, "new") == 0)       lex->tk = LEX_R_NEW;
		else if (strcmp(lex->tkStr->cstr, "typeof") == 0)       lex->tk = LEX_R_TYPEOF;
		else if (strcmp(lex->tkStr->cstr, "throw") == 0)     lex->tk = LEX_R_THROW;
		else if (strcmp(lex->tkStr->cstr, "try") == 0)    	 lex->tk = LEX_R_TRY;
		else if (strcmp(lex->tkStr->cstr, "catch") == 0)     lex->tk = LEX_R_CATCH;
	} else if (is_numeric(lex->currCh)) { // Numbers
		bool isHex = false;
		if (lex->currCh=='0') {
			str_add(lex->tkStr, lex->currCh);
			lex_get_nextch(lex);
		}
		if (lex->currCh=='x') {
			isHex = true;
			str_add(lex->tkStr, lex->currCh);
			lex_get_nextch(lex);
		}
		lex->tk = LEX_INT;
		while (is_numeric(lex->currCh) || (isHex && is_hexadecimal(lex->currCh))) {
			str_add(lex->tkStr, lex->currCh);
			lex_get_nextch(lex);
		}
		if (!isHex && lex->currCh=='.') {
			lex->tk = LEX_FLOAT;
			str_add(lex->tkStr, '.');
			lex_get_nextch(lex);
			while (is_numeric(lex->currCh)) {
				str_add(lex->tkStr, lex->currCh);
				lex_get_nextch(lex);
			}
		}
		// do fancy e-style floating point
		if (!isHex && (lex->currCh=='e'||lex->currCh=='E')) {
			lex->tk = LEX_FLOAT;
			str_add(lex->tkStr, lex->currCh);
			lex_get_nextch(lex);
			if (lex->currCh=='-') {
				str_add(lex->tkStr, lex->currCh);
				lex_get_nextch(lex);
			}
			while (is_numeric(lex->currCh)) {
				str_add(lex->tkStr, lex->currCh);
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
					case 'n' : str_add(lex->tkStr, '\n'); break;
					case 'r' : str_add(lex->tkStr, '\r'); break;
					case 't' : str_add(lex->tkStr, '\t'); break;
					case '"' : str_add(lex->tkStr, '\"'); break;
					case '\\' : str_add(lex->tkStr, '\\'); break;
					default: str_add(lex->tkStr, lex->currCh);
				}
			} else {
				str_add(lex->tkStr, lex->currCh);
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
					case 'n' : str_add(lex->tkStr, '\n'); break;
					case 'a' : str_add(lex->tkStr, '\a'); break;
					case 'r' : str_add(lex->tkStr, '\r'); break;
					case 't' : str_add(lex->tkStr, '\t'); break;
					case '\'' : str_add(lex->tkStr, '\''); break;
					case '\\' : str_add(lex->tkStr, '\\'); break;
					case 'x' : { // hex digits
											 char buf[3] = "??";
											 lex_get_nextch(lex);
											 buf[0] = lex->currCh;
											 lex_get_nextch(lex);
											 buf[1] = lex->currCh;
											 str_add(lex->tkStr, (char)strtol(buf,0,16));
										 } break;
					default: if (lex->currCh>='0' && lex->currCh<='7') {
										 // octal digits
										 char buf[4] = "???";
										 buf[0] = lex->currCh;
										 lex_get_nextch(lex);
										 buf[1] = lex->currCh;
										 lex_get_nextch(lex);
										 buf[2] = lex->currCh;
										 str_add(lex->tkStr, (char)strtol(buf,0,8));
									 } else
										 str_add(lex->tkStr, lex->currCh);
				}
			} else {
				str_add(lex->tkStr, lex->currCh);
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
	str_reset(lex->tkStr);
	lex_get_nextch(lex);
	lex_get_nextch(lex);
	lex_get_next_token(lex);
}

void lex_init(lex_t * lex, const char* input) {
	lex->data = input;
	lex->dataStart = 0;
	lex->dataEnd = strlen(lex->data);
	lex->tkStr = str_new("");
	lex_reset(lex);
}

void lex_release(lex_t* lex) {
	str_free(lex->tkStr);
	lex->tkStr = NULL;
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
		pos= lex->tkLastEnd;

	int l = 1;
	int c  = 1;
	int i;
	for (i=0; i<pos; i++) {
		char ch;
		if (i < lex->dataEnd){
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

	lex_get_pos(l, &line, &col, pos);
	str_reset(ret);
	str_append(ret, "(line: ");
	str_append(ret, str_from_int(line));
	str_append(ret, ", col: ");
	str_append(ret, str_from_int(col));
	str_append(ret, ")");
}

bool lex_chkread(lex_t* lex, uint32_t expected_tk) {
	if (lex->tk != expected_tk) {
#ifdef MARIO_DEBUG
		_debug("Got ");
		_debug(lex_get_token_str(lex->tk));
		_debug(" expected ");
		_debug(lex_get_token_str(expected_tk));
#endif
		str_t* s = str_new("");
		lex_get_pos_str(lex, -1, s);
		_debug(s->cstr);
		str_free(s);
		_debug("!\n");
		return false;
	}
	lex_get_next_token(lex);
	return true;
}



/** JS bytecode.-----------------------------*/

typedef uint16_t OprCode;

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

static int16_t _thisStrIndex = 0;

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

	uint32_t len = strlen(str);
	char* p = (char*)_malloc(len + 1);
	memcpy(p, str, len+1);
	array_add(&bc->strTable, p);
	return sz;
}	

void bc_init(bytecode_t* bc) {
	bc->cindex = 0;
	bc->codeBuf = NULL;
	bc->bufSize = 0;
	array_init(&bc->strTable);

	_thisStrIndex = bc_getstrindex(bc, THIS);
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

#define bc_getstr(bc, i) ((i>=(bc)->strTable.size) ? "" : (const char*)(bc)->strTable.items[i])

/*const char* bc_getstr(bytecode_t* bc, int i) {
	if(i<0 || i == 0xFFFF ||  i>=bc->strTable.size)
		return "";
	return (const char*)bc->strTable.items[i];
}	
*/

PC bc_bytecode(bytecode_t* bc, OprCode instr, const char* str) {
	OprCode r = instr;
	OprCode i = 0xFFFF;

	if(str != NULL && str[0] != 0)
		i = bc_getstrindex(bc, str);

	return INS(r, i);
}
	
PC bc_gen_int(bytecode_t* bc, OprCode instr, int32_t i) {
	PC ins = bc_bytecode(bc, instr, "");
	bc_add(bc, ins);
	bc_add(bc, i);
	return bc->cindex;
}

PC bc_gen_short(bytecode_t* bc, OprCode instr, int32_t s) {
	PC ins = bc_bytecode(bc, instr, "");
	ins = (ins&0xFFFF0000) | (s&0x0FFFF);
	bc_add(bc, ins);
	return bc->cindex;
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
		if(i >= 0 && i < 0xFFFF) //short int
			bc->codeBuf[bc->cindex-1] = INS(INSTR_INT_S, i);
		else 	
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

void bc_set_instr(bytecode_t* bc, PC anchor, OprCode op, PC target) {
	if(target == ILLEGAL_PC)
		target = bc->cindex;

	int offset = target > anchor ? (target-anchor) : (anchor-target);
	PC ins = INS(op, offset);
	bc->codeBuf[anchor] = ins;
}

PC bc_add_instr(bytecode_t* bc, PC anchor, OprCode op, PC target) {
	if(target == ILLEGAL_PC)
		target = bc->cindex;

	int offset = target > anchor ? (target-anchor) : (anchor-target);
	PC ins = INS(op, offset);
	bc_add(bc, ins);
	return bc->cindex;
} 

#ifdef MARIO_DEBUG

const char* instr_str(OprCode ins) {
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
	PC ins = bc->codeBuf[i];
	OprCode instr = (ins >> 16) & 0xFFFF;
	OprCode offset = ins & 0xFFFF;

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

void bc_dump(bytecode_t* bc) {
	PC i;
	char index[8];
	PC sz = bc->strTable.size;

	_debug("-------string table--------------------\n");
	for(i=0; i<sz; ++i) {
		sprintf(index, "%04X: ", i);
		_debug(index);
		_debug((const char*)bc->strTable.items[i]);
		_debug("\n");
	}
	_debug("---------------------------------------\n");

	str_t* s = str_new("");

	i = 0;
	while(i < bc->cindex) {
		i = bc_get_instr_str(bc, i, s);
		_debug(s->cstr);
		_debug("\n");
		i++;
	}
	str_free(s);
	_debug("---------------------------------------\n");
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

void gen_func_name(const char* name, int argNum, str_t* full) {
	str_reset(full);
	str_cpy(full, name);
	if(argNum > 0) {
		str_append(full, "$");
		str_append(full, str_from_int(argNum));
	}
}

int parse_func_name(const char* full, str_t* name) {
	const char* pos = strchr(full, '$');
	int argsNum = 0;
	if(pos != NULL) {
		argsNum = atoi(pos+1);
		if(name != NULL)
			str_ncpy(name, full, pos-full);	
	}
	else {
		if(name != NULL)
			str_cpy(name, full);
	}
	return argsNum;
}

int callFunc(lex_t* l, bytecode_t* bc) {
	if(!lex_chkread(l, '(')) return -1;
	int argNum = 0;
	while(true) {
		PC pc1 = bc->cindex;
		if(!base(l, bc))
			return -1;
		PC pc2 = bc->cindex;
		if(pc2 > pc1) //not empty, means valid arguemnt.
			argNum++;

		if (l->tk!=')') {
			if(!lex_chkread(l, ',')) return -1;	
		}
		else
			break;
	}
	if(!lex_chkread(l, ')')) return -1;
	return argNum;
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
	/* we can have functions without names */
	if (l->tk == LEX_ID) {
		str_cpy(name, l->tkStr->cstr);
		if(!lex_chkread(l, LEX_ID)) return false;
	}
	
	if(l->tk == LEX_ID) { //class get/set token
		if(strcmp(name->cstr, "get") == 0) {
			str_cpy(name, l->tkStr->cstr);
			if(!lex_chkread(l, LEX_ID)) return false;
			bc_gen(bc, INSTR_FUNC_GET);
		}
		if(strcmp(name->cstr, "set") == 0) {
			str_cpy(name, l->tkStr->cstr);
			if(!lex_chkread(l, LEX_ID)) return false;
			bc_gen(bc, INSTR_FUNC_SET);
		}
	}
	else {
		bc_gen(bc, INSTR_FUNC);
	}
	//do arguments
	if(!lex_chkread(l, '(')) return false;
	while (l->tk!=')') {
		bc_gen_str(bc, INSTR_VAR, l->tkStr->cstr);
		if(!lex_chkread(l, LEX_ID)) return false;
		if (l->tk!=')') {
			if(!lex_chkread(l, ',')) return false;
		}
	}
	if(!lex_chkread(l, ')')) return false;
	PC pc = bc_reserve(bc);
	block(l, bc, NULL, true);
	
	OprCode op = bc->codeBuf[bc->cindex - 1] >> 16;

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
		str_cpy(name, l->tkStr->cstr);
		if(!lex_chkread(l, LEX_ID)) return false;
	}
	bc_gen_str(bc, INSTR_CLASS, name->cstr);
	
	/*read extends*/
	if (l->tk==LEX_R_EXTENDS) {
		if(!lex_chkread(l, LEX_R_EXTENDS)) return false;
		str_cpy(name, l->tkStr->cstr);
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
		bc_gen_str(bc, INSTR_INT, l->tkStr->cstr);
		if(!lex_chkread(l, LEX_INT)) return false;
	}
	else if (l->tk==LEX_FLOAT) {
		bc_gen_str(bc, INSTR_FLOAT, l->tkStr->cstr);
		if(!lex_chkread(l, LEX_FLOAT)) return false;
	}
	else if (l->tk==LEX_STR) {
		bc_gen_str(bc, INSTR_STR, l->tkStr->cstr);
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
		str_t* className = str_new("");
		str_cpy(className, l->tkStr->cstr);

		if(!lex_chkread(l, LEX_ID)) return false;
		if (l->tk == '(') {
			//lex_chkread(l, '(');
			int argNum = callFunc(l, bc);
			//lex_chkread(l, ')');
			if(argNum > 0) {
				str_append(className, "$");
				str_append(className, str_from_int(argNum));
			}
			bc_gen_str(bc, INSTR_NEW, className->cstr);
		}
		str_free(className);
	}

	if (l->tk=='{') {
		// JSON-style object definition
		if(!lex_chkread(l, '{')) return false;
		bc_gen(bc, INSTR_OBJ);
		while (l->tk != '}') {
			str_t* id = str_new(l->tkStr->cstr);
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
		str_t* name = str_new(l->tkStr->cstr);
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
					int argNum = callFunc(l, bc);
					gen_func_name((const char*)names.items[sz], argNum, s);
					bc_gen_str(bc, INSTR_CALL, s->cstr);	
				}
				else {
					int i;
					for(i=0; i<sz; i++) {
						bc_gen_str(bc, load ? INSTR_LOAD:INSTR_GET, (const char*)names.items[i]);	
						load = false;
					}
					int argNum = callFunc(l, bc);
					gen_func_name((const char*)names.items[sz], argNum, s);
					bc_gen_str(bc, INSTR_CALLO, s->cstr);	
				}
				load = false;
				array_clean(&names, NULL);
				str_free(s);
			} 
			else if (l->tk == '.') { // ------------------------------------- Record Access
				if(!lex_chkread(l, '.')) return false;
				if(name->len == 0)
					str_cpy(name, l->tkStr->cstr);
				else {
					str_append(name, ".");
					str_append(name, l->tkStr->cstr);
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
				array_clean(&names, NULL);

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
			array_clean(&names, NULL);
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
	OprCode instr = INSTR_END;
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
		OprCode op;
		//bool beConst;

		if(l->tk == LEX_R_VAR) {
			if(!lex_chkread(l, LEX_R_VAR)) return false;
			//beConst = false;
			op = INSTR_VAR;
		}
		else if(l->tk == LEX_R_LET) {
			if(!lex_chkread(l, LEX_R_LET)) return false;
			//beConst = false;
			op = INSTR_LET;
		}
		else {
			if(!lex_chkread(l, LEX_R_CONST)) return false;
			//beConst = true;
			op = INSTR_CONST;
		}

		while (l->tk != ';') {
			str_t* vname = str_new(l->tkStr->cstr);
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
		bc_gen(bc, INSTR_BLOCK);

		if(!lex_chkread(l, '(')) return false;
		PC cpc = bc->cindex; //continue anchor
		if(!base(l, bc)) return false; //condition
		if(!lex_chkread(l, ')')) return false;

		PC pc = bc_reserve(bc); //njmp on condition
		bc_add_instr(bc, pc, INSTR_JMP, pc+2); //jmp to loop(skip the next jump instruction).
		PC pcb = bc_reserve(bc); //jump out of loop (for break anchor);
		
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
		bc_gen(bc, INSTR_BLOCK);

		if(!lex_chkread(l, '(')) return false;
		if(!statement(l, bc, true, NULL)) //init statement
			return false;

		PC cpc = bc->cindex; //condition anchor(also continue anchor as well)
		if(!base(l, bc)) //condition
			return false; 
		if(!lex_chkread(l, ';')) return false;
		PC pc = bc_reserve(bc); //njmp on condition for jump out of loop.
		PC lpc = bc_reserve(bc); //jmp to loop(skip iterator part).

		PC ipc = bc->cindex;  //iterator anchor;
		if(!base(l, bc)) //iterator statement
			return false; 
		if(!lex_chkread(l, ')')) return false;
		bc_gen(bc, INSTR_POP); //pop the stack.
		bc_add_instr(bc, cpc, INSTR_JMPB, ILLEGAL_PC); //jump to coninue anchor;

		PC pcb = bc_reserve(bc); //jump out of loop (for break anchor);

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
			_debug("Error: There is no loop for 'break' here!\n");
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
			_debug("Error: There is no loop for 'continue' here!\n");
			return false;
		}

		bc_gen_short(bc, INSTR_BLOCK_END, loop->blockDepth);
		bc_add_instr(bc, loop->continueAnchor, INSTR_JMPB, ILLEGAL_PC); //to continue anchor;
		pop = false;
	}
	else {
			_debug("Error: don't understand '");
			_debug(l->tkStr->cstr);
			_debug("'!\n");
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
#ifdef MARIO_DEBUG
	bc_dump(bc);
#endif
	return true;
}

/** vm var-----------------------------*/

node_t* node_new(const char* name) {
	node_t* node = (node_t*)_malloc(sizeof(node_t));
	node->magic = 1;

	uint32_t len = strlen(name);
	node->name = (char*)_malloc(len+1);
	memcpy(node->name, name, len+1);
	node->nameID = -1;

	node->beConst = false;
	node->var = var_new();	
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

void var_remove_all(var_t* var) {
	/*free children*/
	array_clean(&var->children, node_free);
}

node_t* var_add(var_t* var, const char* name, var_t* add) {
	node_t* node = node_new(name);
	if(node != NULL) {
		if(add != NULL)
			node_replace(node, add);
		else 
			var_ref(node->var);
		array_add(&var->children, node);
	}
	return node;
}

node_t* var_find(var_t* var, const char*name, int16_t nameID) {
	int i;

	if(nameID >= 0) {
		for(i=0; i<var->children.size; i++) {
			node_t* node = (node_t*)var->children.items[i];
			if(node != NULL) {
				if(node->nameID == nameID)  {
					return node;
				}
			}
		}
	}

	/*_debug(name);
	_debug(" xxx\n");
	*/

	for(i=0; i<var->children.size; i++) {
		node_t* node = (node_t*)var->children.items[i];
		if(node != NULL) {
			if(strcmp(node->name, name) == 0) {
				if(nameID >= 0)
					node->nameID = nameID;
				return node;
			}
		}
	}
	return NULL;
}

node_t* var_find_create(var_t* var, const char*name , int16_t nameID) {
	node_t* n = var_find(var, name, nameID);
	if(n != NULL)
		return n;
	n = var_add(var, name, NULL);
	n->nameID = nameID;
	return n;
}

node_t* var_get(var_t* var, int32_t index) {
	int32_t i;
	for(i=var->children.size; i<=index; i++) {
		var_add(var, "", NULL);
	}

	node_t* node = (node_t*)array_get(&var->children, index);
	return node;
}

void func_free(void* p);

void var_free(void* p) {
	var_t* var = (var_t*)p;
	if(var == NULL || var->refs > 0)
		return;

	/*free children*/
	var_remove_all(var);	

	/*free value*/
	if(var->value != NULL) {
		if(var->freeFunc != NULL) 
			var->freeFunc(var->value);
		else
			_free(var->value);
	}
	
	_free(var);
}

/*
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
	var->size = 0;

	var->value = NULL;
	var->freeFunc = NULL;
	array_init(&var->children);
	return var;
}

var_t* var_new_int(int i) {
	var_t* var = var_new();
	var->type = V_INT;
	var->value = _malloc(sizeof(int));
	*((int*)var->value) = i;
	return var;
}
*/

var_t* var_new_obj(void*p, free_func_t fr) {
	var_t* var = var_new();
	var->type = V_OBJECT;
	var->value = p;
	var->freeFunc = fr;
	return var;
}

var_t* var_new_float(float i) {
	var_t* var = var_new();
	var->type = V_FLOAT;
	var->value = _malloc(sizeof(float));
	*((float*)var->value) = i;
	return var;
}

var_t* var_new_str(const char* s) {
	var_t* var = var_new();
	var->type = V_STRING;
	var->size = strlen(s);
	var->value = _malloc(var->size + 1);
	memcpy(var->value, s, var->size + 1);
	return var;
}

const char* var_get_str(var_t* var) {
	if(var == NULL || var->value == NULL)
		return "";
	
	return (const char*)var->value;
}

int var_get_int(var_t* var) {
	if(var == NULL || var->value == NULL)
		return 0;
	if(var->type == V_FLOAT)	
		return (int)(*(float*)var->value);
	return *(int*)var->value;
}

float var_get_float(var_t* var) {
	if(var == NULL || var->value == NULL)
		return 0.0;
	
	if(var->type == V_INT)	
		return (float)(*(int*)var->value);
	return *(float*)var->value;
}

func_t* var_get_func(var_t* var) {
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

void var_to_str(var_t* var, str_t* ret) {
	str_reset(ret);

	switch(var->type) {
	case V_INT:
		str_cpy(ret, str_from_int(var_get_int(var)));
		break;
	case V_FLOAT:
		str_cpy(ret, str_from_float(var_get_float(var)));
		break;
	case V_STRING:
		str_cpy(ret, var_get_str(var));
		break;
	case V_ARRAY:
	case V_OBJECT:
		var_to_json_str(var, ret, 0);
		break;
	default:
		str_cpy(ret, "undefined");
		break;
	}
}

void get_parsable_str(var_t* var, str_t* ret) {
	str_reset(ret);

	if (var->type == V_FUNC) {
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

	str_t* s = str_new("");
	var_to_str(var, s);
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
void var_to_json_str(var_t* var, str_t* ret, int level) {
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

	if (var->type == V_OBJECT) {
		// children - handle with bracketed list
		int sz = (int)var->children.size;
		if(sz > 0)
			str_append(ret, "{\n");
		else
			str_append(ret, "{");

		int i;
		for(i=0; i<sz; ++i) {
			node_t* n = var_get(var, i);
			append_json_spaces(ret, level);
			str_add(ret, '"');
			str_append(ret, n->name);
			str_add(ret, '"');
			str_append(ret, ": ");

			str_t* s = str_new("");
			var_to_json_str(n->var, s, level+1);
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
	else if (var->type == V_ARRAY) {
		str_add(ret, '[');
		int len = (int)var->children.size;
		if (len>100) len=100; // we don't want to get stuck here!

		int i;
		for (i=0;i<len;i++) {
			node_t* n = var_get(var, i);

			str_t* s = str_new("");
			var_to_json_str(n->var, s, level);
			str_append(ret, s->cstr);
			str_free(s);

			if (i<len-1) 
				str_append(ret, ", ");
		}
		str_add(ret, ']');
	}
	else {
		// no children or a function... just write value directly
		str_t* s = str_new("");
		get_parsable_str(var, s);
		str_append(ret, s->cstr);
		str_free(s);
	}

	if(level == 0) {
		array_remove_all(&done);
	}
}

/** var cache for const value --------------*/

#ifdef MARIO_CACHE

static var_t** _var_cache = NULL;
static uint32_t _var_cache_used = 0;
static uint32_t _var_cache_size = 0;

void var_cache_init(uint16_t size) {
	_var_cache_size = size;	
	_var_cache = (var_t**)_malloc(sizeof(var_t*) * size);
	_var_cache_used = 0;	
}

void var_cache_free() {
	uint32_t i;
	for(i=0; i<_var_cache_used; ++i) {
		var_t* v = _var_cache[i];
		var_unref(v, true);
	}
	_free(_var_cache);
	_var_cache_used = 0;	
}

int32_t var_cache(var_t* v) {
	if(_var_cache_used >= _var_cache_size)
		return -1;
	_var_cache[_var_cache_used] = var_ref(v);
	_var_cache_used++;
	return _var_cache_used-1;
}

bool try_cache(PC* ins, var_t* v) {
	if((*ins) & INSTR_NEED_IMPROVE) {
		int index = var_cache(v); 
		if(index >= 0) 
			*ins = (INSTR_CACHE << 16 | index);
		return true;
	}

	*ins = (*ins) | INSTR_NEED_IMPROVE;
	return false;
}

#define get_cache(index) _var_cache[index]

#endif

/** JSON Parser-----------------------------*/

var_t* json_parse_factor(lex_t *l) {
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
		int i = atoi(l->tkStr->cstr);
		lex_chkread(l, LEX_INT);
		return var_new_int(i);
	}
	else if (l->tk==LEX_FLOAT) {
		float f = atof(l->tkStr->cstr);
		lex_chkread(l, LEX_FLOAT);
		return var_new_float(f);
	}
	else if (l->tk==LEX_STR) {
		str_t* s = str_new(l->tkStr->cstr);
		lex_chkread(l, LEX_STR);
		var_t* ret = var_new_str(s->cstr);
		str_free(s);
		return ret;
	}
	else if(l->tk==LEX_R_FUNCTION) {
		lex_chkread(l, LEX_R_FUNCTION);
		//TODO
		_debug("Error: Can not parse json function item!\n");
		return var_new();
	}
	else if (l->tk=='[') {
		/* JSON-style array */
		var_t* arr = var_new();
		arr->type = V_ARRAY;
		lex_chkread(l, '[');
		while (l->tk != ']') {
			var_t* v = json_parse_factor(l);
			var_add(arr, "", v);
			if (l->tk != ']') 
				lex_chkread(l, ',');
		}
		lex_chkread(l, ']');
		return arr;
	}
	else if (l->tk=='{') {
		lex_chkread(l, '{');
		var_t* obj = var_new();
		obj->type = V_OBJECT;
		while(l->tk != '}') {
			str_t* id = str_new(l->tkStr->cstr);
			if(l->tk == LEX_STR)
				lex_chkread(l, LEX_STR);
			else
				lex_chkread(l, LEX_ID);

			lex_chkread(l, ':');
			var_t* v = json_parse_factor(l);
			var_add(obj, id->cstr, v);
			str_free(id);
			if(l->tk != '}')
				lex_chkread(l, ',');
		}
		lex_chkread(l, '}');
		return obj;
	}
	return var_new();
}

var_t* json_parse(const char* str) {
	lex_t lex;
	lex_init(&lex, str);
	var_t* ret = json_parse_factor(&lex);
	lex_release(&lex);
	return ret;
}

/** Interpreter-----------------------------*/


#define vm_push(vm, var) ({ \
	var_t* __var_ = (var); \
	var_ref(__var_); \
	array_add(&(vm)->stack, __var_);  \
	})

#define vm_push_node(vm, node) ({ \
	node_t* _node_ = node; \
	var_ref(_node_->var); \
	array_add(&(vm)->stack, _node_); })

void vm_pop(vm_t* vm) {
	int index = vm->stack.size-1;
	if(index < 0)
		return;

	int8_t magic = *(int8_t*)vm->stack.items[index];
	if(magic == 0) {//var
		var_t* var = (var_t*)vm->stack.items[index];
		var_unref(var, true);
	}
	else { //node
		node_t* node = (node_t*)vm->stack.items[index];
		if(node != NULL)
			var_unref(node->var, true);
	}
	vm->stack.items[index] = NULL;
	vm->stack.size--;
}

node_t* vm_pop2node(vm_t* vm) {
	int index = vm->stack.size-1;
	if(index < 0)
		return NULL;

	int8_t magic = *(int8_t*)vm->stack.items[index];
	if(magic != 1) //not node!
		return NULL;

	node_t* node = (node_t*)vm->stack.items[index];
	vm->stack.items[index] = NULL;
	vm->stack.size--;
	return node;
}

//var_t* vm_pop2(vm_t* vm) {
#define  vm_pop2(vm) ({\
	int index = (vm)->stack.size-1; \
	if(index < 0) \
		NULL; \
	void* p = (vm)->stack.items[index]; \
	int8_t magic = *(int8_t*)p; \
	var_t* var; \
	if(magic == 1) \
		var = ((node_t*)p)->var; \
	else  \
		var = (var_t*)p; \
	(vm)->stack.items[index] = NULL; \
	(vm)->stack.size--; \
	var; })\

var_t* vm_stack_pick(vm_t* vm, int depth) {
	int index = (int)vm->stack.size-depth;
	if(index < 0)
		return NULL;

	int8_t magic = *(int8_t*)vm->stack.items[index];
	if(magic == 1) {//node
		node_t* node = (node_t*)array_remove(&vm->stack, index);
		return node->var;
	}
	return (var_t*)array_remove(&vm->stack, index);
}

//scope of vm runing
typedef struct st_scope {
	struct st_scope* prev;
	var_t* var;
	PC pc; //stack pc
	bool isBlock;

	//continue and break anchor for loop(while/for)
} scope_t;

scope_t* scope_new(var_t* var, PC pc) {
	scope_t* sc = (scope_t*)_malloc(sizeof(scope_t));
	sc->prev = NULL;

	if(var != NULL)
		sc->var = var_ref(var);
	sc->pc = pc;
	sc->isBlock = false;
	return sc;
}

void scope_free(void* p) {
	scope_t* sc = (scope_t*)p;
	if(sc == NULL)
		return;
	if(sc->var != NULL)
		var_unref(sc->var, true);
	_free(sc);
}

#define vm_get_scope(vm) (scope_t*)array_tail(&(vm)->scopes)

var_t* vm_get_scope_var(vm_t* vm, bool skipBlock) {
	scope_t* sc = (scope_t*)array_tail(&vm->scopes);
	if(skipBlock && sc != NULL && sc->isBlock) //skip blocks
		sc = sc->prev;

	if(sc == NULL)
		return vm->root;
	return sc->var;	
}

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
	array_del(&vm->scopes, vm->scopes.size-1, scope_free);
	return pc;
}

void vm_stack_free(void* p) {
	int8_t magic = *(int8_t*)p;
	if(magic == 1) {//node
		node_t* node = (node_t*)p;
		node_free(node);
	}
	else {
		var_t* var = (var_t*)p;
		var_free(var);
	}
}

node_t* vm_find(vm_t* vm, const char* name, int16_t nameID) {
	var_t* var = vm_get_scope_var(vm, true);
	if(var == NULL)
		return NULL;
	return var_find(var, name, nameID);	
}

node_t* vm_find_in_class(var_t* var, const char* name, int16_t nameID) {
	node_t* n = var_find(var, PROTOTYPE, -1);

	while(n != NULL && n->var != NULL && n->var->type == V_OBJECT) {
		node_t* ret = NULL;
		ret = var_find(n->var, name, nameID);
		if(ret != NULL)
			return ret;
		n = var_find(n->var, SUPER, -1);
	}
	return NULL;
}

node_t* find_member(var_t* obj, const char* name, int16_t nameID) {
	node_t* node = var_find(obj, name, nameID);
	if(node == NULL) { 
		node = vm_find_in_class(obj, name, nameID);
	}
	return node;
}

node_t* vm_find_in_scopes(vm_t* vm, const char* name, int16_t nameID) {
	node_t* ret = NULL;
	scope_t* sc = vm_get_scope(vm);
	
	if(sc != NULL && sc->var != NULL) {
		ret = var_find(sc->var, name, nameID);
		if(ret != NULL)
			return ret;

		node_t* n = var_find(sc->var, THIS, -1);//_thisStrIndex);
		if(n != NULL)  {
			ret = find_member(n->var, name, nameID);
			if(ret != NULL)
				return ret;
		}
		sc = sc->prev;
	}

	while(sc != NULL) {
		if(sc->var != NULL) {
			ret = var_find(sc->var, name, nameID);
			if(ret != NULL)
				return ret;
		}
		sc = sc->prev;
	}

	return var_find(vm->root, name, nameID);
}

node_t* vm_load_node(vm_t* vm, const char* name, int16_t nameID, bool create) {
	node_t* n =  vm_find_in_scopes(vm, name, nameID);	
	if(n != NULL)
		return n;
	/*
	_debug("Warning: '");	
	_debug(name);
	_debug("' undefined!\n");	
	*/
	if(!create)
		return NULL;

	var_t* var = vm_get_scope_var(vm, true);
	if(var == NULL)
		return NULL;

	n =var_add(var, name, NULL);	
	n->nameID = nameID;
	return n;
}

//for function.

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

void func_free(void* p) {
	func_t* func = (func_t*)p;
	array_clean(&func->args, NULL);
	_free(p);
}

var_t* var_new_func(func_t* func) {
	var_t* var = var_new();
	var->type = V_FUNC;
	var->freeFunc = func_free;
	var->value = func;
	return var;
}

var_t* find_func(vm_t* vm, var_t* obj, const char* fname) {
	//try full name with argNum
	node_t* node;
	if(obj != NULL) {
		node = find_member(obj, fname, -1);
	}
	else {
		node = vm_find_in_scopes(vm, fname, -1);
	}

	if(node != NULL && node->var != NULL && node->var->type == V_FUNC) {
		return node->var;
	}
	return NULL;
}

var_t* get_super(var_t* var) {
	if(var == NULL)
		return NULL;

	node_t* n = var_find(var, SUPER, -1);
	if(n != NULL)
		return n->var;
	return NULL;
}

void vm_run_code(vm_t* vm);
bool func_call(vm_t* vm, var_t* obj, func_t* func, int argNum) {
	int i;
	var_t *env = var_new();
	env->type = V_ARRAY;

	for(i=argNum; i>func->args.size; i--) {
		vm_pop(vm);
	}

	for(i=func->args.size-1; i>=0; i--) {
		const char* argName = (const char*)array_get(&func->args, i);
		var_t* v = NULL;
		if(i >= argNum) {
			v = var_new();
			var_ref(v);
		}
		else {
			v = vm_pop2(vm);	
		}	
		if(v != NULL) {
			var_add(env, argName, v);
			var_unref(v, true);
		}
	}
	var_add(env, THIS, obj);

	var_t* super = get_super(func->owner);
	if(super != NULL)
		var_add(env, SUPER, super);

	scope_t* sc = scope_new(env, vm->pc);
	vm_push_scope(vm, sc);

	//native function
	if(func->native != NULL) {
		var_t* ret = func->native(vm, env, func->data);
		if(ret == NULL)
			ret = var_new();
		vm_push(vm, ret);
		vm_pop_scope(vm);
		return true;
	}

	//js function
	vm->pc = func->pc;
	vm_run_code(vm);
	return true;
}

var_t* func_def(vm_t* vm, bool regular) {
	func_t* func = func_new();
	func->regular = regular;
	while(true) {
		PC ins = vm->bc.codeBuf[vm->pc++];
		OprCode instr = ins >> 16;
		OprCode offset = ins & 0x0000FFFF;
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

	var_t* ret = var_new_func(func);
	return ret;
}

void math_op(vm_t* vm, OprCode op, var_t* v1, var_t* v2) {
	if(v1->value == NULL || v2->value == NULL) {
		vm_push(vm, var_new());
		return;
	}	

	if((v1->type == V_INT || v1->type == V_FLOAT) && 
			(v2->type == V_INT || v2->type == V_FLOAT)) {
		//do number 
		float f1, f2, ret = 0.0;
		bool floatMode = false;
		if(v1->type == V_FLOAT || v2->type == V_FLOAT)
			floatMode = true;

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
			case INSTR_MOD: 
			case INSTR_MODEQ: 
				ret = (((int)f1) % (int)f2);
				break; 
			case INSTR_RSHIFT: 
				ret = (((int)f1) >> (int)f2);
				break; 
			case INSTR_LSHIFT: 
				ret = (((int)f1) << (int)f2);
				break; 
			case INSTR_AND: 
				ret = (((int)f1) & (int)f2);
				break; 
			case INSTR_OR: 
				ret = (((int)f1) | (int)f2);
				break; 
		}

		var_t* v;
		if(op == INSTR_PLUSEQ || 
				op == INSTR_MINUSEQ ||
				op == INSTR_DIVEQ ||
				op == INSTR_MULTIEQ ||
				op == INSTR_MODEQ)  {
			v = v1;
			if(floatMode) 
				*(float*)v->value = ret;
			else 
				*(int*)v->value = (int)ret;
		}
		else {
			if(floatMode) 
				v = var_new_float(ret);
			else 
				v = var_new_int((int)ret);
		}
		vm_push(vm, v);
		return;
	}

	//do string + 
	if(op == INSTR_PLUS || op == INSTR_PLUSEQ) {
		str_t* s = str_new((const char*)v1->value);
		switch(v2->type) {
			case V_STRING: 
				str_append(s, (const char*)v2->value);
				break;
			case V_INT: 
				str_append(s, str_from_int(*(int*)v2->value));
				break;
			case V_FLOAT: 
				str_append(s, str_from_float(*(float*)v2->value));
				break;
			/*
			case BCVar::ARRAY: 
			case BCVar::OBJECT: 
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

void compare(vm_t* vm, OprCode op, var_t* v1, var_t* v2) {
	float f1, f2;
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
	var_t* ret = var_new_int(i);
	vm_push(vm, ret);
}

void do_get(vm_t* vm, var_t* v, const char* name, int16_t nameID) {
	if(v->type == V_STRING && strcmp(name, "length") == 0) {
		int len = strlen(var_get_str(v));
		vm_push(vm, var_new_int(len));
		return;
	}
	else if(v->type == V_ARRAY && strcmp(name, "length") == 0) {
		int len = v->children.size;
		vm_push(vm, var_new_int(len));
		return;
	}	

	node_t* n = find_member(v, name, nameID);
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
			n = var_add(v, name, NULL);
		else {
			_debug("Can not get member '");
			_debug(name);
			_debug("'!\n");
			n = node_new(name);
			vm_push(vm, var_new());
			return;
		}
	}

	vm_push_node(vm, n);
}

void doExtends(vm_t* vm, var_t* cls, const char* superName) {
	node_t* n = vm_find_in_scopes(vm, superName, -1);
	if(n == NULL) {
		_debug("Super Class '");
		_debug(superName);
		_debug("' not found!\n");
		return;
	}
	var_add(cls, SUPER, n->var);
}

/** simple create object by classname(no constructor) */
var_t* new_obj(vm_t* vm, const char* clsName, int argNum) {
	var_t* obj = NULL;

	node_t* n = vm_load_node(vm, clsName, -1, false); //load class;

	if(n == NULL || n->var->type != V_OBJECT) {
		_debug("Error: There is no class: '");
		_debug(clsName);
		_debug("'!\n");
		return NULL;
	}

	obj = var_new_obj(NULL, NULL);
	var_add(obj, PROTOTYPE, n->var);

	n = var_find(n->var, CONSTRUCTOR, -1);

	if(n != NULL) {
		func_call(vm, obj, (func_t*)n->var->value, argNum);
		obj = vm_pop2(vm);
		var_unref(obj, false);
	}

	return obj;
}

/** create object and try constructor */
void do_new(vm_t* vm, const char* full) {
	str_t* name = str_new("");
	int argNum = parse_func_name(full, name);
	(void)argNum;

	var_t* obj = new_obj(vm, name->cstr, argNum);
	str_free(name);

	if(obj == NULL)
		vm_push(vm, var_new());
	else
		vm_push(vm, obj);
}

void vm_run_code(vm_t* vm) {
	//int32_t scDeep = vm->scopes.size;
	PC codeSize = vm->bc.cindex;
	PC* code = vm->bc.codeBuf;

	while(vm->pc < codeSize) {
		PC ins = code[vm->pc++];
		OprCode instr = OP(ins);
		OprCode offset = ins & 0x0000FFFF;

		if(instr == INSTR_END)
			break;
		
		switch(instr) {
			case INSTR_NIL: {	break; }
			case INSTR_BLOCK: 
			{
				scope_t* bl = vm_get_scope(vm);
				scope_t* sc = NULL;
				if(bl != NULL)
					sc = scope_new(bl->var, bl->pc);
				else
					sc = scope_new(var_new_obj(NULL, NULL), 0xFFFFFFFF);
				sc->isBlock = true;
				vm_push_scope(vm, sc);
				break;
			}
			case INSTR_BLOCK_END: 
			{
				int i;
				for(i=0; i<offset; i++) {
					vm_pop_scope(vm);
				}	
				break;
			}
			#ifdef MARIO_CACHE
			case INSTR_CACHE: 
			{	
				var_t* v = get_cache(offset);
				vm_push(vm, v);
				break;
			}
			#endif
			case INSTR_TRUE: 
			{
				var_t* v = var_new_int(1);	
				#ifdef MARIO_CACHE
				try_cache(&code[vm->pc-1], v);
				#endif
				vm_push(vm, v);
				break;
			}
			case INSTR_FALSE: 
			{
				var_t* v = var_new_int(0);	
				#ifdef MARIO_CACHE
				try_cache(&code[vm->pc-1], v);
				#endif
				vm_push(vm, v);
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
					var_unref(v, true);
				}
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
					var_unref(v, true);
				}
				break;
			}
			case INSTR_NOT: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					int i = 0;
					if(v->type == V_UNDEF || *(int*)v->value == 0)
						i = 1;
					var_unref(v, true);
					vm_push(vm, var_new_int(i));
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
			case INSTR_GEQ: 
			{
				var_t* v2 = vm_pop2(vm);
				var_t* v1 = vm_pop2(vm);
				if(v1 != NULL && v2 != NULL) {
					compare(vm, instr, v1, v2);
					var_unref(v1, true);
					var_unref(v2, true);
				}
				break;
			}
			case INSTR_AAND: 
			case INSTR_OOR: 
			{
				var_t* v2 = vm_pop2(vm);
				var_t* v1 = vm_pop2(vm);
				if(v1 != NULL && v2 != NULL) {
					int r = 0;
					int i1 = *(int*)v1->value;
					int i2 = *(int*)v2->value;

					if(instr == INSTR_AAND)
						r = (i1 != 0) && (i2 != 0);
					else
						r = (i1 != 0) || (i2 != 0);
					vm_push(vm, var_new_int(r));

					var_unref(v1, true);
					var_unref(v2, true);
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
					var_unref(v1, true);
					var_unref(v2, true);
				}
				break;
			}
			case INSTR_MMINUS_PRE: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					int *i = (int*)v->value;
					(*i)--;
					if((ins & INSTR_NEED_IMPROVE) == 0) {
						if(OP(code[vm->pc]) != INSTR_POP) { 
							vm_push(vm, v);
						}
						else { code[vm->pc] = INSTR_NIL; code[vm->pc-1] |= INSTR_NEED_IMPROVE; }
					}
					var_unref(v, true);
				}
				break;
			}
			case INSTR_MMINUS: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					int *i = (int*)v->value;
					if((ins & INSTR_NEED_IMPROVE) == 0) {
						var_t* v2 = var_new_int(*i);
						if(OP(code[vm->pc]) != INSTR_POP) {
							vm_push(vm, v2);
						}
						else { 
							code[vm->pc] = INSTR_NIL; 
							code[vm->pc-1] |= INSTR_NEED_IMPROVE;
							var_unref(v2, true);
						}
					}
					(*i)--;
					var_unref(v, true);
				}
				break;
			}
			case INSTR_PPLUS_PRE: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					int *i = (int*)v->value;
					(*i)++;
					
					if((ins & INSTR_NEED_IMPROVE) == 0) {
						if(OP(code[vm->pc]) != INSTR_POP) { 
							vm_push(vm, v);
						}
						else { code[vm->pc] = INSTR_NIL; code[vm->pc-1] |= INSTR_NEED_IMPROVE; }
					}
					var_unref(v, true);
				}
				break;

			}
			case INSTR_PPLUS: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					int *i = (int*)v->value;
					if((ins & INSTR_NEED_IMPROVE) == 0) {
						var_t* v2 = var_new_int(*i);
						if(OP(code[vm->pc]) != INSTR_POP) {
							vm_push(vm, v2);
						}
						else { 
							code[vm->pc] = INSTR_NIL;
							code[vm->pc-1] |= INSTR_NEED_IMPROVE; 
							var_unref(v2, true);
						}
					}

					(*i)++;
					var_unref(v, true);
				}
				break;
			}
			case INSTR_RETURN:  //return without value
			case INSTR_RETURNV: 
			{ //return with value
				scope_t* sc = vm_get_scope(vm);
				if(sc != NULL) {
					if(instr == INSTR_RETURN) {//return without value, push "this" to stack
						node_t* n = vm_load_node(vm, THIS, _thisStrIndex, false); //load variable.
						if(n != NULL)
							vm_push(vm, n->var);
						else
							vm_push(vm, var_new());
					}
					vm->pc = sc->pc;
					vm_pop_scope(vm);
				}
				return;
			}
			case INSTR_VAR:
			case INSTR_CONST: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				node_t *node = vm_find(vm, s, offset);
				if(node != NULL) { //find just in current scope
					_debug("Warning: '");
					_debug(s);
					_debug("' has already existed!\n");
				}
				else {
					var_t* v = vm_get_scope_var(vm, true);
					if(v != NULL) {
						node = var_add(v, s, NULL);
						node->nameID = offset;
					}
					if(node != NULL && instr == INSTR_CONST)
						node->beConst = true;
				}
				break;
			}
			case INSTR_LET:
			{
				const char* s = bc_getstr(&vm->bc, offset);
				var_t* v = vm_get_scope_var(vm, false);
				node_t *node = var_find(v, s, offset);
				if(node != NULL) { //find just in current scope
					_debug("Warning: '");
					_debug(s);
					_debug("' has already existed!\n");
				}
				else {
					node = var_add(v, s, NULL);
				}
				break;
			}
			case INSTR_INT:
			{
				var_t* v = var_new_int((int)code[vm->pc++]);
				#ifdef MARIO_CACHE
				if(try_cache(&code[vm->pc-2], v))
					code[vm->pc-1] = INSTR_NIL;
				#endif
				vm_push(vm, v);
				break;
			}
			case INSTR_INT_S:
			{
				var_t* v = var_new_int(offset);
				#ifdef MARIO_CACHE
				try_cache(&code[vm->pc-1], v);
				#endif
				vm_push(vm, v);
				break;
			}
			case INSTR_FLOAT: 
			{
				var_t* v = var_new_float(*(float*)(&code[vm->pc++]));
				#ifdef MARIO_CACHE
				if(try_cache(&code[vm->pc-2], v))
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
				try_cache(&code[vm->pc-1], v);
				#endif
				vm_push(vm, v);
				break;
			}
			case INSTR_ASIGN: 
			{
				var_t* v = vm_pop2(vm);
				node_t* n = vm_pop2node(vm);
				if(v != NULL && n != NULL) {
					bool modi = (!n->beConst || n->var->type == V_UNDEF);
					var_unref(n->var, true);
					if(modi) 
						node_replace(n, v);
					else {
						_debug("Can not change a const variable: '");
						_debug(n->name);
						_debug("'!\n");
					}
					var_unref(v, true);

					if((ins & INSTR_NEED_IMPROVE) == 0) {
						if(OP(code[vm->pc]) != INSTR_POP) {
							vm_push(vm, n->var);
						}
						else { code[vm->pc] = INSTR_NIL; code[vm->pc-1] |= INSTR_NEED_IMPROVE; }
					}
				}
				break;
			}
			case INSTR_LOAD: 
			{
				bool loaded = false;
				if(offset == _thisStrIndex) {
					var_t* v = vm_get_scope_var(vm, true);
					if(v->type == V_OBJECT) {
						vm_push(vm, v);
						loaded = true;
					}
				}

				if(!loaded) {
					const char* s = bc_getstr(&vm->bc, offset);
					node_t* n = vm_load_node(vm, s, offset, true); //load variable, create if not exist.
					//node_t* n = vm_load_node(vm, s, -1, true); //load variable, create if not exist.
					vm_push_node(vm, n);
				}
				break;
			}
			case INSTR_GET: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					do_get(vm, v, s, offset);
					var_unref(v, true);
				}
				else {
					vm_push(vm, var_new());
					_debug("Error: can not find member '");
					_debug(s);
					_debug("'!\n");
				}
				break;
			}
			case INSTR_CALL: 
			case INSTR_CALLO: 
			{
				var_t* func = NULL;
				var_t* obj = NULL;
				const char* s = bc_getstr(&vm->bc, offset);
				str_t* name = str_new("");
				int argNum = parse_func_name(s, name);
				
				if(instr == INSTR_CALLO) {
					obj = vm_stack_pick(vm, argNum+1);
				}

				if(obj == NULL && strcmp(SUPER, name->cstr) == 0) { //super constructor
					str_cpy(name, CONSTRUCTOR);
					var_t* v = vm_get_scope_var(vm, true);
					if(v != NULL) {
						node_t* n = var_find(v, THIS, _thisStrIndex);
						if(n != NULL)
							obj = var_ref(n->var);
						n = var_find(v, SUPER, -1);
						if(n != NULL)
							func = find_func(vm, n->var, name->cstr);
					}
				}
				else {
					func = find_func(vm, obj, name->cstr);
				}
				str_free(name);

				if(func != NULL) {
					func_call(vm, obj, (func_t*)func->value, argNum);
				}
				else {
					while(argNum > 0) {
						vm_pop(vm);
						argNum--;
					}
					vm_push(vm, var_new());
					_debug("Error: can not find function '");
					_debug(s);
					_debug("'!\n");
				}
				var_unref(obj, true);
				break;
			}
			case INSTR_MEMBER: 
			case INSTR_MEMBERN: 
			{
				const char* s = (instr == INSTR_MEMBER ? "" :  bc_getstr(&vm->bc, offset));

				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					var_t *var = vm_get_scope_var(vm, true);

					if(v->type == V_FUNC) {
						func_t* func = (func_t*)v->value;
						func->owner = var;
					}
					if(var != NULL) {
						node_t* n = var_add(var, s, v);
						if(instr == INSTR_MEMBERN)
							n->nameID = offset;
					}	

					var_unref(v, true);
				}
				break;
			}

			case INSTR_FUNC: 
			case INSTR_FUNC_GET: 
			case INSTR_FUNC_SET: 
			{
				var_t* v = func_def(vm, (instr == INSTR_FUNC ? true:false));
				if(v != NULL)
					vm_push(vm, v);
				break;
			}
			case INSTR_OBJ:
			case INSTR_ARRAY: 
			{
				var_t* obj = var_new();
				if(instr == INSTR_OBJ)
					obj->type =V_OBJECT;
				else
					obj->type = V_ARRAY;
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
					var_unref(v2, true);

					node_t* n = var_get(v1, at);
					if(n != NULL) {
						vm_push_node(vm, n);
					}
					var_unref(v1, true);
				}
				break;
			}
			case INSTR_CLASS: 
			{
				const char* s =  bc_getstr(&vm->bc, offset);
				node_t* n = var_find_create(vm->root, s, offset);
				n->var->type = V_OBJECT;
				//read extends
				ins = code[vm->pc];
				instr = ins >> 16;
				if(instr == INSTR_EXTENDS) {
					vm->pc++;
					offset = ins & 0x0000FFFF;
					s =  bc_getstr(&vm->bc, offset);
					doExtends(vm, n->var, s);
				}

				scope_t* sc = scope_new(n->var, 0xFFFFFFFF);
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
			case INSTR_NEW: 
			{
				const char* s =  bc_getstr(&vm->bc, offset);
				do_new(vm, s);
				break;
			}
			/*
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
}

bool vm_load(vm_t* vm, const char* s) {
	return compile(&vm->bc, s);
}

bool vm_run(vm_t* vm) {
	vm_run_code(vm);
	return true;
}

void vm_close(vm_t* vm) {
	var_unref(vm->root, true);

	#ifdef MARIO_CACHE
	var_cache_free();
	#endif

	array_clean(&vm->scopes, NULL);	
	array_clean(&vm->stack, vm_stack_free);	
	bc_release(&vm->bc);
}	

/** native extended functions.-----------------------------*/

node_t* vm_new_class(vm_t* vm, const char* cls) {
	node_t* clsNode = vm_load_node(vm, cls, -1, true);
	clsNode->var->type = V_OBJECT;
	return clsNode;
}

node_t* vm_reg_var(vm_t* vm, const char* cls, const char* name, var_t* var, bool beConst) {
	var_t* clsvar = vm->root;
	if(cls[0] != 0) {
		node_t* clsnode = vm_new_class(vm, cls);
		clsvar = clsnode->var;
	}

	node_t* node = var_add(clsvar, name, var);
	node->beConst = beConst;
	return node;
}

node_t* vm_reg_native(vm_t* vm, const char* cls, const char* decl, native_func_t native, void* data) {
	var_t* clsVar = vm->root;
	if(cls[0] != 0) {
		node_t* clsNode = vm_new_class(vm, cls);
		clsVar = clsNode->var;
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

	var_t* var = var_new_func(func);
	node_t* node = var_add(clsVar, name->cstr, var);
	str_free(name);

	return node;
}

const char* get_str(var_t* var, const char* name) {
	node_t* n = var_find(var, name, -1);
	return n == NULL ? "" : var_get_str(n->var);
}

int get_int(var_t* var, const char* name) {
	node_t* n = var_find(var, name, -1);
	return n == NULL ? 0 : var_get_int(n->var);
}

float get_float(var_t* var, const char* name) {
	node_t* n = var_find(var, name, -1);
	return n == NULL ? 0.0 : var_get_float(n->var);
}

var_t* get_obj(var_t* var, const char* name) {
	node_t* n = var_find(var, name, -1);
	if(n == NULL)
		return NULL;
	return n->var;
}

/**dump variable*/
var_t* native_dump(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "var", -1);
	if(n == NULL)
		return NULL;

	str_t* s = str_new("");
	var_to_json_str(n->var, s, 0);
	_debug(s->cstr);
	str_free(s);

	_debug("\n");
	return NULL;
}

/**print string*/
var_t* native_print(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	const char* s = get_str(env, "str");
	_debug(s);
	return NULL;
}

/**println string*/
var_t* native_println(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	const char* s = get_str(env, "str");
	_debug(s);
	_debug("\n");
	return NULL;
}

#define VAR_CACHE_MAX 32

void vm_init(vm_t* vm) {
	vm->pc = 0;
	bc_init(&vm->bc);
	array_init(&vm->stack);	
	array_init(&vm->scopes);	

	#ifdef MARIO_CACHE
	var_cache_init(VAR_CACHE_MAX);
	#endif

	vm->root = var_new_obj(NULL, NULL);
	var_ref(vm->root);

	vm_reg_native(vm, "Debug", "dump(var)", native_dump, NULL);
	vm_reg_native(vm, "Debug", "print(str)", native_print, NULL);
	vm_reg_native(vm, "", "print(str)", native_print, NULL);
	vm_reg_native(vm, "", "println(str)", native_println, NULL);
	vm_reg_native(vm, "", "dump(var)", native_dump, NULL);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

