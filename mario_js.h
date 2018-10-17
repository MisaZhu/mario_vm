/**
very tiny js engine in single file.
*/

#ifndef MARIO_JS
#define MARIO_JS

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#else

#ifndef bool
typedef enum {false, true} bool;
#endif

#endif


#include <inttypes.h>
#include <string.h>
#include <stdio.h>

/** memory functions.-----------------------------*/
#ifndef PRE_ALLOC
#include <stdlib.h>
	#define _malloc malloc
	#define _realloc realloc
	#define _free free
#else
/*TODO*/
#endif

#ifndef null
#define null NULL
#endif

typedef void (*free_func_t)(void* p);

extern void _free_none(void*p);

extern void (*_debug_func)(const char*);
void _debug(const char* s);

typedef struct st_array {
	void** items;
	uint32_t max: 16;
	uint32_t size: 16;
} m_array_t;

void array_init(m_array_t* array);
void array_add(m_array_t* array, void* item);
void* array_add_buf(m_array_t* array, void* s, uint32_t sz);
void* array_get(m_array_t* array, uint32_t index);
void* array_set(m_array_t* array, uint32_t index, void* p);
void* array_tail(m_array_t* array);
void* array_head(m_array_t* array);
void* array_remove(m_array_t* array, uint32_t index);
void array_del(m_array_t* array, uint32_t index, free_func_t fr);
void array_remove_all(m_array_t* array);

void array_clean(m_array_t* array, free_func_t fr);

typedef struct st_str {
	char* cstr;
	uint32_t max: 16;
	uint32_t len: 16;
} str_t;

void str_reset(str_t* str);
char* str_ncpy(str_t* str, const char* src, uint32_t l);
char* str_cpy(str_t* str, const char* src);
str_t* str_new(const char* s);
char* str_append(str_t* str, const char* src);
char* str_add(str_t* str, char c);
void str_free(str_t* str);
const char* str_from_int(int i);
const char* str_from_float(float i);
int str_to_int(const char* str);
float str_to_float(const char* str);
void str_split(const char* str, char c, m_array_t* array);

typedef uint32_t PC;
typedef struct st_bytecode {
	PC cindex;
	m_array_t strTable;
	PC *codeBuf;
	uint32_t bufSize;
} bytecode_t;

//script var
#define V_UNDEF  0
#define V_INT    1
#define V_FLOAT  2
#define V_STRING 3
#define V_OBJECT 4
#define V_BLOCK  5

#define THIS "this"
#define PROTOTYPE "prototype"
#define SUPER "super"
#define CONSTRUCTOR "constructor"

typedef struct st_var {
	uint32_t magic: 8; //0 for var; 1 for node
	uint32_t refs:18;
	uint32_t type:4;
	uint32_t isArray:1;
	uint32_t isFunc:1;
	uint32_t size;  // size for bytes type of value;

	void* value;
	free_func_t freeFunc; //how to free value
	free_func_t onDestroy; //before destroyed.

	m_array_t children;
} var_t;

struct st_vm;
typedef var_t* (*native_func_t)(struct st_vm *, var_t*, void*);

typedef struct st_func {
	native_func_t native;
	bool regular;
	PC pc;
	void *data;
	m_array_t args; //argument names
	var_t* owner;
} func_t;

//script node for var member children
typedef struct st_node {
	int16_t magic: 8; //1 for node
  int16_t beConst : 8;
	char* name;
	var_t* var;
} node_t;


#ifdef MARIO_THREAD
#include <pthread.h>

typedef struct st_isignal {
	var_t* obj;
	var_t* handleFunc;
	var_t* args;
	struct st_isignal* next;
} isignal_t;
#endif


#define VM_STACK_MAX 32

typedef struct st_vm {
	bytecode_t bc;
	m_array_t scopes;
	void* stack[VM_STACK_MAX];
	int32_t stackTop;
	PC pc;

	bool terminated;
	var_t* root;

	m_array_t initNatives;
	m_array_t closeNatives;

	#ifdef MARIO_THREAD
	pthread_mutex_t interruptLock;
	isignal_t* isignalHead;
	isignal_t* isignalTail;
	uint32_t isignalNum;
	bool interrupted;
	#endif
} vm_t;


node_t* node_new(const char* name);
void node_free(void* p);
var_t* node_replace(node_t* node, var_t* v);

void var_dump(var_t* var);
void var_remove_all(var_t* var);
node_t* var_add(var_t* var, const char* name, var_t* add);
node_t* var_find(var_t* var, const char*name);
var_t* var_find_var(var_t* var, const char*name);
node_t* var_find_create(var_t* var, const char*name);
node_t* var_get(var_t* var, int32_t index);

void var_free(void* p);

var_t* var_ref(var_t* var);
void var_unref(var_t* var, bool del);

//#define var_ref(var) ({ ++(var)->refs; var; })
//#define var_unref(var, del) ({ --(var)->refs; if((var)->refs <= 0 && (del)) var_free((var)); })

var_t* var_new();
var_t* var_new_block();
var_t* var_new_array();
var_t* var_new_int(int i);
var_t* var_new_obj(void*p, free_func_t fr);
var_t* var_new_float(float i);
var_t* var_new_str(const char* s);
const char* var_get_str(var_t* var);
int var_get_int(var_t* var);
float var_get_float(var_t* var);
func_t* var_get_func(var_t* var);

void var_to_json_str(var_t*, str_t*, int);
var_t* json_parse(const char* str);

void vm_init(vm_t* vm);
bool vm_load(vm_t* vm, const char* s);
bool vm_run(vm_t* vm);
void vm_close(vm_t* vm);

var_t* new_obj(vm_t* vm, const char* clsName, int argNum);
node_t* vm_find(vm_t* vm, const char* name);
node_t* vm_find_in_class(var_t* var, const char* name);
node_t* vm_reg_var(vm_t* vm, const char* cls, const char* name, var_t* var, bool beConst);
node_t* vm_reg_native(vm_t* vm, const char* cls, const char* decl, native_func_t native, void* data);
void vm_reg_init(vm_t* vm, void (*func)(void*), void* data);
void vm_reg_close(vm_t* vm, void (*func)(void*), void* data);

node_t* find_member(var_t* obj, const char* name);
var_t* get_obj(var_t* obj, const char* name);
const char* get_str(var_t* obj, const char* name);
int get_int(var_t* obj, const char* name);
float get_float(var_t* obj, const char* name);

var_t* callJSFunc(vm_t* vm, var_t* obj, var_t* func, var_t* args);
var_t* callJSFuncByName(vm_t* vm, var_t* obj, const char* funcName, var_t* args);


#ifdef MARIO_THREAD
bool interrupt(vm_t* vm, var_t* obj, var_t* func, var_t* args);
bool interruptByName(vm_t* vm, var_t* obj, const char* funcName, var_t* args);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
