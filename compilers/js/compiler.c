/**
very tiny script engine in single file.
*/

#include "mario_vm.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

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

	char s[STATIC_STR_MAX];
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
		char s[STATIC_STR_MAX];
		str_append(full, str_from_int(arg_num, s));
	}
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
				char s[STATIC_STR_MAX];
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

bool compiler(bytecode_t *bc, const char* input) {
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

