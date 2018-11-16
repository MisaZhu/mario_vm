/**
very tiny script engine in single file.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "mario_bc.h"
#include <stdlib.h>
#include <stdio.h>

/** bytecode.-----------------------------*/

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

void bc_init(bytecode_t* bc) {
	bc->cindex = 0;
	bc->code_buf = NULL;
	bc->buf_size = 0;
	array_init(&bc->str_table);
}

void bc_release(bytecode_t* bc) {
	array_clean(&bc->str_table, NULL);
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
		case  INSTR_BLOCK_LOOP	: return "BLOCKL";
		case  INSTR_BLOCK_TRY		: return "BLOCKT";
		case  INSTR_BLOCK_END		: return "BLOCKE";
		case  INSTR_THROW				: return "THROW";
		case  INSTR_TRY					: return "TRY";
		case  INSTR_CATCH				: return "CATCH";
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
				instr == INSTR_INT_S) {
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

#ifdef __cplusplus
}
#endif /* __cplusplus */
