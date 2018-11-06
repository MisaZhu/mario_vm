/**
demo mario_vm compiler.
*/

#include "mario_vm.h"
#include "mario_lex.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Script Lex. -----------------------------*/

typedef enum {
  // reserved words
  LEX_R_VAR = LEX_BASIC_END,
} LEX_TYPES;

void lex_get_next_token(lex_t* lex) {
	lex->tk = LEX_EOF;
	str_reset(lex->tk_str);

	lex_skip_space(lex); //skip the space like ' ','\t','\r'. but keep '\n' for end of sentence.
	//skip comments.
	if(lex_skip_comments_line(lex, ";")) { //line comments
		lex_get_next_token(lex);
		return;
	}
	if(lex_skip_comments_block(lex, "/*", "*/")) { //block comments
		lex_get_next_token(lex);
		return;
	}

	lex_token_start(lex); //
	//get basic tokens like LEX_INT, LEX_FLOAT, LEX_STR, LEX_ID, or LEX_EOF if got nothing.
	lex_get_basic_token(lex);

	if (lex->tk == LEX_ID) { //  IDs
		//try to replace LEX_ID token by reserved word.
		if (strcmp(lex->tk_str->cstr, "var")  == 0) 
			lex->tk = LEX_R_VAR;
	} 
	if(lex->tk == LEX_EOF) {
		//simplely set token with single char
		lex_get_char_token(lex);
	}
	lex_token_end(lex);
}

/**check current token with expected one, and read the next token*/
bool lex_chkread(lex_t* lex, uint32_t expected_tk) {
	if(lex->tk != expected_tk) 
		return false;

	lex_get_next_token(lex);
	return true;
}

/*function */
void gen_func_name(const char* name, int arg_num, str_t* full) {
	str_reset(full);
	str_cpy(full, name);
	if(arg_num > 0) {
		str_append(full, "$");
		char s[STATIC_STR_MAX];
		str_append(full, str_from_int(arg_num, s));
	}
}

bool base(lex_t* l, bytecode_t* bc);

int call_func(lex_t* l, bytecode_t* bc) {
	if(!lex_chkread(l, '(')) return -1;
	int arg_num = 0;
	while(true) { // parameters.
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

void factor_func(lex_t* l, bytecode_t* bc, const char* name) {
	int arg_num = call_func(l, bc);
	str_t* fname = str_new("");
	gen_func_name(name, arg_num, fname);
	bc_gen_str(bc, INSTR_CALL, fname->cstr);	
	str_free(fname);
}

bool factor(lex_t* l, bytecode_t* bc) {
	if (l->tk=='(') {
		if(!lex_chkread(l, '(')) return false;
		if(!base(l, bc)) return false;
		if(!lex_chkread(l, ')')) return false;
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
	else if(l->tk==LEX_ID) {
		str_t* name = str_new(l->tk_str->cstr);
		if(!lex_chkread(l, LEX_ID)) return false;

		if (l->tk=='(') { // ------------------------------------- Function Call
			factor_func(l, bc, name->cstr);
		} 
		else {
			bc_gen_str(bc, INSTR_LOAD, name->cstr);
		}
		str_free(name);
	}

	return true;
}

bool term(lex_t* l, bytecode_t* bc) {
	if(!factor(l, bc))
		return false;

	while (l->tk=='*' || l->tk=='/') {
		LEX_TYPES op = l->tk;
		if(!lex_chkread(l, l->tk)) return false;
		if(!factor(l, bc)) return false;

		if(op == '*') {
			bc_gen(bc, INSTR_MULTI);
		}
		else if(op == '/') {
			bc_gen(bc, INSTR_DIV);
		}
	}

	return true;	
}

bool expr(lex_t* l, bytecode_t* bc) {
	LEX_TYPES pre = l->tk;

	if (l->tk=='-') {
		if(!lex_chkread(l, '-')) return false;
	}

	if(!term(l, bc))
		return false;

	if (pre == '-') {
		bc_gen(bc, INSTR_NEG);
	}

	while (l->tk=='+' || l->tk=='-') {
		int op = l->tk;
		if(!lex_chkread(l, l->tk))
			return false;
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

bool base(lex_t* l, bytecode_t* bc) {
	if(!expr(l, bc))
		return false;

	if (l->tk=='=') {
		LEX_TYPES op = l->tk;
		if(!lex_chkread(l, l->tk)) return false;
		base(l, bc);
		// sort out initialiser
		if (op == '=')  {
			bc_gen(bc, INSTR_ASIGN);
		}
	}
	return true;
}

bool statement(lex_t* l, bytecode_t* bc) {
	bool pop = true;
	if(l->tk == '\n') {
		pop = false;
		if(!lex_chkread(l, '\n')) return false;
	}
	else if (l->tk==LEX_ID    ||
			l->tk==LEX_INT   ||
			l->tk==LEX_FLOAT ||
			l->tk==LEX_STR   ||
			l->tk=='-'    ) {
		/* Execute a simple statement that only contains basic arithmetic... */
		if(!base(l, bc))
			return false;
		if(!lex_chkread(l, '\n')) return false;
	}
	else if (l->tk==LEX_R_VAR) {
		pop = false;

		if(!lex_chkread(l, LEX_R_VAR)) return false;

		str_t* vname = str_new(l->tk_str->cstr);
		if(!lex_chkread(l, LEX_ID)) return false;
		bc_gen_str(bc, LEX_R_VAR, vname->cstr);
		// sort out initialiser
		if (l->tk == '=') {
			if(!lex_chkread(l, '=')) return false;
			bc_gen_str(bc, INSTR_LOAD, vname->cstr);
			if(!base(l, bc)) return false;
			bc_gen(bc, INSTR_ASIGN);
			bc_gen(bc, INSTR_POP);
		}
		str_free(vname);
		if(!lex_chkread(l, '\n')) return false;
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
	lex_get_next_token(&lex);

	while(lex.tk) {
		if(!statement(&lex, bc)) {
			lex_release(&lex);
			return false;
		}
	}
	bc_gen(bc, INSTR_END);
	lex_release(&lex);
	return true;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

