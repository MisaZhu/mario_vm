/**
very tiny js engine in single file.
*/

#ifndef MARIO_BC
#define MARIO_BC

#include "mario_utils.h"

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif

//bytecode
typedef uint32_t PC;
typedef uint16_t opr_code_t;
typedef struct st_bytecode {
	PC cindex;
	m_array_t str_table;
	PC *code_buf;
	uint32_t buf_size;
} bytecode_t;

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


PC bc_gen(bytecode_t* bc, opr_code_t instr);
PC bc_gen_str(bytecode_t* bc, opr_code_t instr, const char* s);
PC bc_gen_int(bytecode_t* bc, opr_code_t instr, int32_t i);
PC bc_gen_short(bytecode_t* bc, opr_code_t instr, int32_t i);
void bc_set_instr(bytecode_t* bc, PC anchor, opr_code_t op, PC target);
uint16_t bc_getstrindex(bytecode_t* bc, const char* str);
PC bc_add_instr(bytecode_t* bc, PC anchor, opr_code_t op, PC target);
PC bc_reserve(bytecode_t* bc);

#define bc_getstr(bc, i) (((i)>=(bc)->str_table.size) ? "" : (const char*)(bc)->str_table.items[(i)])

void bc_dump(bytecode_t* bc);
void bc_init(bytecode_t* bc);
void bc_release(bytecode_t* bc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
