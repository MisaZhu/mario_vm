/**
very tiny script engine in single file.
*/

#include "mario_vm.h"
#include "mario_lex.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Script Lex. -----------------------------*/

typedef enum {
  LEX_ID = 256,
  LEX_INT,
  LEX_FLOAT,
  LEX_STR,
  // reserved words
  LEX_R_VAR,
} LEX_TYPES;

void lex_get_next_token(lex_t* lex) {
	lex->tk = LEX_EOF;
	str_reset(lex->tk_str);

	lex_skip_space(lex);
	if(lex_skip_comments_line(lex, "//"))
		return lex_get_next_token(lex);
	if(lex_skip_comments_block(lex, "/*", "*/"))
		return lex_get_next_token(lex);

	// record beginning of this token(pre-read 2 chars );
	lex->tk_start = lex->data_pos-2;
	// tokens
	if (is_alpha(lex->curr_ch)) { //alpha char
		while (is_alpha(lex->curr_ch) || is_numeric(lex->curr_ch)) {
			str_add(lex->tk_str, lex->curr_ch);
			lex_get_nextch(lex);
		}
		lex->tk = LEX_ID; //lex_id first, and continue to try other reserved.

		if (strcmp(lex->tk_str->cstr, "var")  == 0) 
			lex->tk = LEX_R_VAR;
	} 
	else if (is_numeric(lex->curr_ch)) { // _numbers
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
	}
	else if (lex->curr_ch=='"') {
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
	} else {
		// single char
		lex->tk = (LEX_TYPES)lex->curr_ch;
		if (lex->curr_ch) 
			lex_get_nextch(lex);
	}
	lex->tk_last_end = lex->tk_end;
	lex->tk_end = lex->data_pos-3;
}

bool lex_chkread(lex_t* lex, uint32_t expected_tk) {
	if(lex->tk != expected_tk) 
		return false;

	lex_get_next_token(lex);
	return true;
}

bool base(lex_t* l, bytecode_t* bc);

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

		bc_gen_str(bc, INSTR_LOAD, name->cstr);
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

