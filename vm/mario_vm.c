/**
very tiny script engine in single file.
*/

#include "mario_vm.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** vm var-----------------------------*/

node_t* node_new(const char* name) {
	node_t* node = (node_t*)_malloc(sizeof(node_t));
	node->magic = 1;

	uint32_t len = strlen(name);
	node->name = (char*)_malloc(len+1);
	memcpy(node->name, name, len+1);

	node->be_const = false;
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

inline var_t* node_replace(node_t* node, var_t* v) {
	if(node->var->type == V_INT && v->type == V_INT) {
		*(int*)(node->var->value) = *(int*)(v->value);
	}
	else if(node->var->type == V_FLOAT && v->type == V_FLOAT) {
		*(float*)(node->var->value) = *(float*)(v->value);
	}
	else {
		var_t* old = node->var;
		node->var = var_ref(v);
		var_unref(old, true);
	}
	return node->var;
}

inline void var_remove_all(var_t* var) {
	/*free children*/
	array_clean(&var->children, node_free);
}

node_t* var_add(var_t* var, const char* name, var_t* add) {
	node_t* node = NULL;

	if(name[0] != 0) 
		node = var_find(var, name);

	if(node == NULL) {
		node = node_new(name);
		var_ref(node->var);
		array_add(&var->children, node);
	}

	if(add != NULL)
		node_replace(node, add);

	return node;
}

inline node_t* var_find(var_t* var, const char*name) {
	uint32_t i;

	for(i=0; i<var->children.size; i++) {
		node_t* node = (node_t*)var->children.items[i];
		if(node != NULL) {
			if(strcmp(node->name, name) == 0) {
				return node;
			}
		}
	}
	return NULL;
}

inline var_t* var_find_var(var_t* var, const char*name) {
	node_t* node = var_find(var, name);
	if(node == NULL)
		return NULL;
	return node->var;
}

inline node_t* var_find_create(var_t* var, const char*name) {
	node_t* n = var_find(var, name);
	if(n != NULL)
		return n;
	n = var_add(var, name, NULL);
	return n;
}

node_t* var_get(var_t* var, int32_t index) {
	node_t* node = (node_t*)array_get(&var->children, index);
	return node;
}

node_t* var_array_get(var_t* var, int32_t index) {
	var_t* arr_var = var_find_var(var, "_ARRAY_");
	if(arr_var == NULL)
		return NULL;

	int32_t i;
	for(i=arr_var->children.size; i<=index; i++) {
		var_add(arr_var, "", NULL);
	}

	node_t* node = (node_t*)array_get(&arr_var->children, index);
	return node;
}

node_t* var_array_set(var_t* var, int32_t index, var_t* set_var) {
	node_t* node = var_array_get(var, index);
	if(node != NULL)
		node_replace(node, set_var);
	return node;
}

uint32_t var_array_size(var_t* var) {
	var_t* arr_var = var_find_var(var, "_ARRAY_");
	if(arr_var == NULL)
		return 0;
	return arr_var->children.size;
}

void func_free(void* p);

inline void var_clean(var_t* var) {
	if(var == NULL)
		return;

	/*free children*/
	if(var->children.size > 0)
		var_remove_all(var);	

	/*free value*/
	if(var->value != NULL) {
		if(var->free_func != NULL) 
			var->free_func(var->value);
		else
			_free(var->value);
		var->value = NULL;
	}

	if(var->on_destroy != NULL) {
		var->on_destroy(var);
	}
}

inline void var_free(void* p) {
	var_t* var = (var_t*)p;
	if(var == NULL || var->refs > 0)
		return;
	var_clean(var);
	_free(var);
}

inline var_t* var_ref(var_t* var) {
	//	if(var != NULL)
	++var->refs;
	return var;
}

inline void var_unref(var_t* var, bool del) {
	//	if(var != NULL) {
	if(var->refs > 0)
		--var->refs;

	if(var->refs == 0 && del)
		var_free(var);
	//	}
}

const char* get_typeof(var_t* var) {
	switch(var->type) {
		case V_UNDEF:
			return "undefined";
		case V_INT:
		case V_FLOAT:
			return "number";
		case V_BOOL: 
			return "boolean";
		case V_STRING: 
			return "string";
		case V_OBJECT: 
			return var->is_func ? "function": "object";
	}
	return "undefined";
}

inline var_t* var_new() {
	var_t* var = (var_t*)_malloc(sizeof(var_t));
	var->magic = 0;
	var->refs = 0;
	var->type = V_UNDEF;
	var->size = 0;

	var->value = NULL;
	var->free_func = NULL;
	var->on_destroy = NULL;
	var->is_class = 0;
	var->is_array = 0;
	var->is_func = 0;
	array_init(&var->children);
	return var;
}

inline var_t* var_new_block() {
	var_t* var = var_new();
	var->type = V_BLOCK;
	return var;
}

inline var_t* var_new_array() {
	var_t* var = var_new_obj(NULL, NULL);
	var->is_array = 1;
	var_t* members = var_new_obj(NULL, NULL);
	var_add(var, "_ARRAY_", members);
	return var;
}

inline var_t* var_new_int(int i) {
	var_t* var = var_new();
	var->type = V_INT;
	var->value = _malloc(sizeof(int));
	*((int*)var->value) = i;
	return var;
}

inline var_t* var_new_bool(bool b) {
	var_t* var = var_new();
	var->type = V_BOOL;
	var->value = _malloc(sizeof(int));
	*((int*)var->value) = b;
	return var;
}

inline var_t* var_new_obj(void*p, free_func_t fr) {
	var_t* var = var_new();
	var->type = V_OBJECT;
	var->value = p;
	var->free_func = fr;
	return var;
}

inline var_t* var_new_float(float i) {
	var_t* var = var_new();
	var->type = V_FLOAT;
	var->value = _malloc(sizeof(float));
	*((float*)var->value) = i;
	return var;
}

var_t* get_prototype(var_t* var) {
	return get_obj(var, PROTOTYPE);
}

inline var_t* var_new_str(const char* s) {
	var_t* var = var_new();
	var->type = V_STRING;
	var->size = strlen(s);
	var->value = _malloc(var->size + 1);
	memcpy(var->value, s, var->size + 1);
	return var;
}

inline var_t* var_new_str2(const char* s, uint32_t len) {
	var_t* var = var_new();
	var->type = V_STRING;
	var->size = strlen(s);
	if(var->size > len)
		var->size = len;
	var->value = _malloc(var->size + 1);
	memcpy(var->value, s, var->size + 1);
	((char*)(var->value))[var->size] = 0;
	return var;
}

inline const char* var_get_str(var_t* var) {
	if(var == NULL || var->value == NULL)
		return "";
	
	return (const char*)var->value;
}

inline bool var_get_bool(var_t* var) {
	if(var == NULL || var->value == NULL)
		return false;
	int i = (int)(*(int*)var->value);
	return i==0 ? false:true;
}

inline int var_get_int(var_t* var) {
	if(var == NULL || var->value == NULL)
		return 0;
	if(var->type == V_FLOAT)	
		return (int)(*(float*)var->value);
	return *(int*)var->value;
}

inline float var_get_float(var_t* var) {
	if(var == NULL || var->value == NULL)
		return 0.0;
	
	if(var->type == V_INT)	
		return (float)(*(int*)var->value);
	return *(float*)var->value;
}

inline func_t* var_get_func(var_t* var) {
	if(var == NULL || var->value == NULL)
		return NULL;
	
	return (func_t*)var->value;
}

void get_m_str(const char* str, str_t* ret) {
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
	if(var == NULL) {
		str_cpy(ret, "undefined");
		return;
	}

	char s[STATIC_STR_MAX];
	switch(var->type) {
	case V_INT:
		str_cpy(ret, str_from_int(var_get_int(var), s));
		break;
	case V_FLOAT:
		str_cpy(ret, str_from_float(var_get_float(var), s));
		break;
	case V_STRING:
		str_cpy(ret, var_get_str(var));
		break;
	case V_OBJECT:
		var_to_json_str(var, ret, 0);
		break;
	case V_BOOL:
		str_cpy(ret, var_get_int(var) == 1 ? "true":"false");
		break;
	default:
		str_cpy(ret, "undefined");
		break;
	}
}

void get_parsable_str(var_t* var, str_t* ret) {
	str_reset(ret);

	str_t* s = str_new("");
	var_to_str(var, s);
	if(var->type == V_STRING)
		get_m_str(s->cstr, ret);
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

	if (var->is_array) {
		str_add(ret, '[');
		uint32_t len = var_array_size(var);
		if (len>100) len=100; // we don't want to get stuck here!

		uint32_t i;
		for (i=0;i<len;i++) {
			node_t* n = var_array_get(var, i);

			str_t* s = str_new("");
			var_to_json_str(n->var, s, level);
			str_append(ret, s->cstr);
			str_free(s);

			if (i<len-1) 
				str_append(ret, ", ");
		}
		str_add(ret, ']');
	}
	else if (var->is_func) {
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
	else if (var->type == V_OBJECT) {
		// children - handle with bracketed list
		int sz = (int)var->children.size;
		if(sz > 0)
			str_append(ret, "{\n");
		else
			str_append(ret, "{");

		int i;
		bool had = false;
		for(i=0; i<sz; ++i) {
			node_t* n = var_get(var, i);
			if(strcmp(n->name, "prototype") == 0)
				continue;
			if(had)
				str_append(ret, ",\n");
			had = true;
			append_json_spaces(ret, level);
			str_add(ret, '"');
			str_append(ret, n->name);
			str_add(ret, '"');
			str_append(ret, ": ");

			str_t* s = str_new("");
			var_to_json_str(n->var, s, level+1);
			str_append(ret, s->cstr);
			str_free(s);
		}
		if(sz > 0) {
			str_add(ret, '\n');
		}
	
		append_json_spaces(ret, level - 1);	
		str_add(ret, '}');
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

static inline var_t* vm_load_var(vm_t* vm, const char* name, bool create) {
	node_t* n = vm_load_node(vm, name, create);
	if(n != NULL)
		return n->var;
	return NULL;
}

static inline void vm_load_basic_classes(vm_t* vm) {
	vm->var_String = vm_load_var(vm, "String", false);
	vm->var_Array = vm_load_var(vm, "Array", false);
}

static inline var_t* var_build_basic_prototype(vm_t* vm, var_t* var) {
	var_t* protoV = get_prototype(var);
	if(protoV != NULL)
		return var;

	var_t* cls_var = NULL;
	if(var->type == V_STRING) { //get basic native class
		cls_var  = vm->var_String;
	}
	else if(var->is_array) {
		cls_var  = vm->var_Array;
	}

	if(cls_var != NULL) {
		protoV = get_prototype(cls_var); //set prototype of var
		var_add(var, PROTOTYPE, protoV);
	}
	return var;
}

/** var cache for const value --------------*/

#ifdef MARIO_CACHE

void var_cache_init(vm_t* vm) {
	uint32_t i;
	for(i=0; i<vm->var_cache_used; ++i) {
		vm->var_cache[i] = NULL;
	}
	vm->var_cache_used = 0;	
}

void var_cache_free(vm_t* vm) {
	uint32_t i;
	for(i=0; i<vm->var_cache_used; ++i) {
		var_t* v = vm->var_cache[i];
		var_unref(v, true);
		vm->var_cache[i] = NULL;
	}
	vm->var_cache_used = 0;	
}

int32_t var_cache(vm_t* vm, var_t* v) {
	if(vm->var_cache_used >= VAR_CACHE_MAX)
		return -1;
	vm->var_cache[vm->var_cache_used] = var_ref(v);
	vm->var_cache_used++;
	return vm->var_cache_used-1;
}

bool try_cache(vm_t* vm, PC* ins, var_t* v) {
	if((*ins) & INSTR_OPT_CACHE) {
		int index = var_cache(vm, v); 
		if(index >= 0) 
			*ins = INS(INSTR_CACHE, index);
		return true;
	}

	*ins = (*ins) | INSTR_OPT_CACHE;
	return false;
}

#endif


/** Interpreter-----------------------------*/

inline void vm_push(vm_t* vm, var_t* var) {  
	var_ref(var);
	if(vm->stack_top < VM_STACK_MAX) {
		vm->stack[vm->stack_top++] = var; 
	} 
}

inline void vm_push_node(vm_t* vm, node_t* node) {
	var_ref(node->var); 
	if(vm->stack_top < VM_STACK_MAX)
		vm->stack[vm->stack_top++] = node;
}

static inline var_t* vm_pop2(vm_t* vm) {
	void *p = NULL;
	vm->stack_top--;
	p = vm->stack[vm->stack_top];
	int8_t magic = *(int8_t*)p;
	//var
	if(magic == 0) {
		return(var_t*)p;
	}

	//node
	node_t* node = (node_t*)p;
	if(node != NULL)
		return node->var;

	return NULL;
}

/*#define vm_pop2(vm) ({ \
	(vm)->stack_top--; \
	void* p = (vm)->stack[(vm)->stack_top]; \
	int8_t magic = *(int8_t*)p; \
	var_t* ret = NULL; \
	if(magic == 0) { \
		ret = (var_t*)p; \
	} \
	else { \
	node_t* node = (node_t*)p; \
	if(node != NULL) \
		ret = node->var; \
	else \
		ret = NULL; \
	} \
	ret; \
})
*/

static inline bool vm_pop(vm_t* vm) {
	if(vm->stack_top == 0)
		return false;

	vm->stack_top--;
	void *p = vm->stack[vm->stack_top];
	int8_t magic = *(int8_t*)p;
	var_t* v;
	if(magic == 0) { //var
		v = (var_t*)p;
	}
	else { //node
		node_t* node = (node_t*)p;
		v = node->var;
	}

	var_unref(v, true);
	return true;
}

static inline node_t* vm_pop2node(vm_t* vm) {
	void *p = NULL;
	/*if(vm->stack_top <= 0) 
		return NULL;
	*/

	vm->stack_top--;
	p = vm->stack[vm->stack_top];
	int8_t magic = *(int8_t*)p;
	if(magic != 1) {//not node!
		return NULL;
	}

	return (node_t*)p;
}

var_t* vm_stack_pick(vm_t* vm, int depth) {
	int index = vm->stack_top - depth;
	if(index < 0)
		return NULL;

	void* p = vm->stack[index];
	var_t* ret = NULL;
	if(p == NULL)
		return ret;

	int8_t magic = *(int8_t*)p;
	if(magic == 1) {//node
		node_t* node = (node_t*)p;
		ret = node->var;
	}
	else {
		ret = (var_t*)p;
	}

	vm->stack_top--;
	int i;
	for(i=index; i<vm->stack_top; ++i) {
		vm->stack[i] = vm->stack[i+1];
	}
	return ret;
}

//scope of vm runing
typedef struct st_scope {
	struct st_scope* prev;
	var_t* var;
	PC pc_start; // continue anchor for loop
	PC pc; // try cache anchor , or break anchor for loop
	int32_t is_func: 16;
	int32_t is_try: 8;
	int32_t is_loop: 8;
	//continue and break anchor for loop(while/for)
} scope_t;

scope_t* scope_new(var_t* var) {
	scope_t* sc = (scope_t*)_malloc(sizeof(scope_t));
	sc->prev = NULL;

	if(var != NULL)
		sc->var = var_ref(var);
	sc->pc = 0;
	sc->pc_start = 0;
	sc->is_func = false;
	sc->is_try = false;
	sc->is_loop = false;
	return sc;
}

scope_t* scope_clone(scope_t* src) {
	scope_t* sc = (scope_t*)_malloc(sizeof(scope_t));
	memcpy(sc, src, sizeof(scope_t));
	if(src->var != NULL)
		sc->var = var_ref(src->var);
	sc->prev = NULL;
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

#define vm_get_scope(vm) (scope_t*)array_tail((vm)->scopes)

#define vm_get_scope_var(vm, skipBlock) ({ \
	scope_t* sc = (scope_t*)array_tail((vm)->scopes); \
	if((skipBlock) && sc != NULL && sc->var->type == V_BLOCK) \
		sc = sc->prev; \
	var_t* ret; \
	if(sc == NULL) \
		ret = (vm)->root; \
	else \
		ret = sc->var; \
	ret; \
})

void vm_push_scope(vm_t* vm, scope_t* sc) {
	scope_t* prev = NULL;
	if(vm->scopes->size > 0)
		prev = (scope_t*)array_tail(vm->scopes);
	array_add(vm->scopes, sc);	
	sc->prev = prev;
}

PC vm_pop_scope(vm_t* vm) {
	if(vm->scopes->size <= 0)
		return 0;

	PC pc = 0;
	scope_t* sc = vm_get_scope(vm);
	if(sc == NULL)
		return 0;

	if(sc->is_func)
		pc = sc->pc;
	array_del(vm->scopes, vm->scopes->size-1, scope_free);
	return pc;
}

void vm_stack_free(vm_t* vm, void* p) {
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

node_t* vm_find(vm_t* vm, const char* name) {
	var_t* var = vm_get_scope_var(vm, true);
	if(var == NULL)
		return NULL;
	return var_find(var, name);	
}

node_t* vm_find_in_class(var_t* var, const char* name) {
	node_t* n = var_find(var, PROTOTYPE);

	while(n != NULL && n->var != NULL && n->var->type == V_OBJECT) {
		node_t* ret = NULL;
		ret = var_find(n->var, name);
		if(ret != NULL)
			return ret;
		n = var_find(n->var, SUPER);
	}
	return NULL;
}

node_t* find_member(var_t* obj, const char* name) {
	node_t* node = var_find(obj, name);
	if(node == NULL) { 
		node = vm_find_in_class(obj, name);
	}
	return node;
}

static inline node_t* vm_find_in_scopes(vm_t* vm, const char* name) {
	node_t* ret = NULL;
	scope_t* sc = vm_get_scope(vm);
	
	/*
	if(sc != NULL && sc->var != NULL) {
		ret = var_find(sc->var, name);
		if(ret != NULL)
			return ret;

		node_t* n = var_find(sc->var, THIS);
		if(n != NULL)  {
			ret = find_member(n->var, name);
			if(ret != NULL)
				return ret;
		}
		sc = sc->prev;
	}
	*/

	while(sc != NULL) {
		if(sc->var != NULL) {
			ret = var_find(sc->var, name);
			if(ret != NULL)
				return ret;
		}
		sc = sc->prev;
	}

	return var_find(vm->root, name);
}

static inline var_t* vm_this_in_scopes(vm_t* vm) {
	node_t* n = vm_find_in_scopes(vm, THIS);
	if(n == NULL)
		return NULL;
	return n->var;
}

inline node_t* vm_load_node(vm_t* vm, const char* name, bool create) {
	var_t* var = vm_get_scope_var(vm, true);

	node_t* n;
	if(var != NULL) {
		n = find_member(var, name);
	}
	else {
		n = vm_find_in_scopes(vm, name);
	}

	if(n == NULL)
		n =  vm_find_in_scopes(vm, name);	

	if(n != NULL)
		return n;
	/*
	_err("Warning: '");	
	_err(name);
	_err("' undefined!\n");	
	*/
	if(!create)
		return NULL;

	if(var == NULL)
		return NULL;

	n =var_add(var, name, NULL);	
	return n;
}

//for function.

var_t* add_prototype(vm_t* vm, var_t* var, var_t* father) {
	var_t* v = var_new_obj(NULL, NULL);
	node_t* protoN = var_add(var, PROTOTYPE, v); //add prototype object

	if(father != NULL) {
		v = get_obj(father, PROTOTYPE); //get father's prototype.
		if(v != NULL) {
			var_add(protoN->var, SUPER, v);
		}
	}

	return protoN->var;
}

func_t* func_new() {
	func_t* func = (func_t*)_malloc(sizeof(func_t));
	if(func == NULL)
		return NULL;
	
	func->native = NULL;
	func->regular = true;
	func->pc = 0;
	func->data = NULL;
	func->owner = NULL;
	func->scopes = NULL;
	array_init(&func->args);
	return func;
}

void func_free(void* p) {
	func_t* func = (func_t*)p;
	array_clean(&func->args, NULL);
	if(func->scopes != NULL)
		array_free(func->scopes, scope_free);
	_free(p);
}

var_t* var_new_func(vm_t* vm, func_t* func) {
	var_t* var = var_new_obj(NULL, NULL);
	var->is_func = 1;
	var->free_func = func_free;
	var->value = func;

	add_prototype(vm, var, vm->var_Object);
	//var_t* protoV = add_prototype(var, vm->var_Object);
	//var_add(protoV, CONSTRUCTOR, var);
	return var;
}

var_t* find_func(vm_t* vm, var_t* obj, const char* fname) {
	//try full name with arg_num
	node_t* node = NULL;
	if(obj != NULL) {
		node = find_member(obj, fname);
	}
	if(node == NULL) {
		node = vm_find_in_scopes(vm, fname);
	}

	if(node != NULL && node->var != NULL && node->var->type == V_OBJECT) {
		return node->var;
	}
	return NULL;
}

bool func_call(vm_t* vm, var_t* obj, var_t* func_var, int arg_num) {
	var_t *env = var_new();
	var_ref(env);
	func_t* func = var_get_func(func_var);
	if(obj == NULL) {
		obj = vm->root;
	}
	var_add(env, THIS, obj);

	int32_t i;
	for(i=arg_num; i>func->args.size; i--) {
		vm_pop(vm);
	}

	for(i=(int32_t)func->args.size-1; i>=0; i--) {
		const char* arg_name = (const char*)array_get(&func->args, i);
		var_t* v = NULL;
		if(i >= arg_num) {
			v = var_new();
			var_ref(v);
		}
		else {
			v = vm_pop2(vm);	
		}	
		if(v != NULL) {
			var_add(env, arg_name, v);
			var_unref(v, true);
		}
	}

	if(func->owner != NULL) {
		node_t* superN = var_find(func->owner, SUPER);
		if(superN != NULL)
			var_add(env, SUPER, superN->var);
	}

	if(func->native != NULL) { //native function
		var_t* ret = func->native(vm, env, func->data);
		if(ret == NULL)
			ret = var_new();
		vm_push(vm, ret);
	}
	else {
		scope_t* sc = scope_new(env);
		sc->pc = vm->pc;
		sc->is_func = true;
		vm_push_scope(vm, sc);

		//script function
		vm->pc = func->pc;
		vm_run(vm);
	}
	var_unref(env, true);
	return true;
}

var_t* func_def(vm_t* vm, bool regular, bool is_static) {
	func_t* func = func_new();
	func->regular = regular;
	func->is_static = is_static;
	while(true) {
		PC ins = vm->bc.code_buf[vm->pc++];
		opr_code_t instr = OP(ins);
		uint32_t offset = OFF(ins);
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

	var_t* ret = var_new_func(vm, func);
	return ret;
}

void vm_mark_func_scopes(vm_t* vm, var_t* func) {
	func_t* f = var_get_func(func);
	if(f == NULL)
		return;

	if(f->scopes != NULL)
		array_free(f->scopes, scope_free);
	
	f->scopes = array_new();
	int32_t i;
	scope_t* prev = NULL;
	for(i=0; i<vm->scopes->size; ++i) {
		scope_t* sc = scope_clone((scope_t*)array_get(vm->scopes, i));
		array_add(f->scopes, sc);
		sc->prev = prev;
		prev = sc;
	}
}

static inline void math_op(vm_t* vm, opr_code_t op, var_t* v1, var_t* v2) {
	/*if(v1->value == NULL || v2->value == NULL) {
		vm_push(vm, var_new());
		return;
	}	
	*/

	//do int
	if(v1->type == V_INT && v2->type == V_INT) {
		int i1, i2, ret = 0;
		i1 = *(int*)v1->value;
		i2 = *(int*)v2->value;

		switch(op) {
			case INSTR_PLUS: 
			case INSTR_PLUSEQ: 
				ret = (i1 + i2);
				break; 
			case INSTR_MINUS: 
			case INSTR_MINUSEQ: 
				ret = (i1 - i2);
				break; 
			case INSTR_DIV: 
			case INSTR_DIVEQ: 
				ret = (i1 / i2);
				break; 
			case INSTR_MULTI: 
			case INSTR_MULTIEQ: 
				ret = (i1 * i2);
				break; 
			case INSTR_MOD: 
			case INSTR_MODEQ: 
				ret = i1 % i2;
				break; 
			case INSTR_RSHIFT: 
				ret = i1 >> i2;
				break; 
			case INSTR_LSHIFT: 
				ret = i1 << i2;
				break; 
			case INSTR_AND: 
				ret = i1 & i2;
				break; 
			case INSTR_OR: 
				ret = i1 | i2;
				break; 
		}

		var_t* v;
		if(op == INSTR_PLUSEQ || 
				op == INSTR_MINUSEQ ||
				op == INSTR_DIVEQ ||
				op == INSTR_MULTIEQ ||
				op == INSTR_MODEQ)  {
			v = v1;
			*(int*)v->value = ret;
		}
		else {
			v = var_new_int(ret);
		}
		vm_push(vm, v);
		return;
	}

	//do float
	if(v1->type == V_FLOAT || v2->type == V_FLOAT) {
		float f1, f2, ret = 0.0;

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
		}

		var_t* v;
		if(op == INSTR_PLUSEQ || 
				op == INSTR_MINUSEQ ||
				op == INSTR_DIVEQ ||
				op == INSTR_MULTIEQ ||
				op == INSTR_MODEQ)  {
			v = v1;
			*(float*)v->value = ret;
		}
		else {
			v = var_new_float(ret);
		}
		vm_push(vm, v);
		return;
	}

	//do string + 
	if(op == INSTR_PLUS || op == INSTR_PLUSEQ) {
		str_t* s = str_new((const char*)v1->value);
		char sfrom[STATIC_STR_MAX];
		switch(v2->type) {
			case V_STRING: 
				str_append(s, var_get_str(v2));
				break;
			case V_INT: 
				str_append(s, str_from_int(var_get_int(v2), sfrom));
				break;
			case V_FLOAT: 
				str_append(s, str_from_float(var_get_float(v2), sfrom));
				break;
			case V_BOOL: 
				str_append(s, str_from_bool(var_get_bool(v2)));
				break;
			/*
			case BC_var::ARRAY: 
			case BC_var::OBJECT: 
				ostr << s << v2->getJSON();
				break;
			*/
		}

		var_t* v;
		if(op == INSTR_PLUSEQ) {
			v = v1;
			char* p = (char*)v->value;
			v->value = _malloc(s->len+1);
			memcpy(v->value, s->cstr, s->len+1);
			if(p != NULL)
				_free(p);
		}
		else {
			v = var_new_str(s->cstr);
		}
		str_free(s);
		vm_push(vm, v);
	}
}

static inline void compare(vm_t* vm, opr_code_t op, var_t* v1, var_t* v2) {
	//do int
	if(v1->type == V_INT && v2->type == V_INT) {
		register int i1, i2;
		i1 = *(int*)v1->value;
		i2 = *(int*)v2->value;

		bool i = false;
		switch(op) {
			case INSTR_EQ: 
			case INSTR_TEQ:
				i = (i1 == i2);
				break; 
			case INSTR_NEQ: 
			case INSTR_NTEQ:
				i = (i1 != i2);
				break; 
			case INSTR_LES: 
				i = (i1 < i2);
				break; 
			case INSTR_GRT: 
				i = (i1 > i2);
				break; 
			case INSTR_LEQ: 
				i = (i1 <= i2);
				break; 
			case INSTR_GEQ: 
				i = (i1 >= i2);
				break; 
		}
		if(i)
			vm_push(vm, vm->var_true);
		else
			vm_push(vm, vm->var_false);
		return;
	}

	
	register float f1, f2;
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

	if(i)
		vm_push(vm, vm->var_true);
	else
		vm_push(vm, vm->var_false);
}

void do_get(vm_t* vm, var_t* v, const char* name) {
	if(v->type == V_STRING && strcmp(name, "length") == 0) {
		int len = strlen(var_get_str(v));
		vm_push(vm, var_new_int(len));
		return;
	}
	else if(v->is_array && strcmp(name, "length") == 0) {
		int len = var_array_size(v);
		vm_push(vm, var_new_int(len));
		return;
	}	

	node_t* n = find_member(v, name);
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
			_err("Can not get member '");
			_err(name);
			_err("'!\n");
			n = node_new(name);
			vm_push(vm, var_new());
			return;
		}
	}

	vm_push_node(vm, n);
}

void doExtends(vm_t* vm, var_t* clsProto, const char* super_name) {
	node_t* n = vm_find_in_scopes(vm, super_name);
	if(n == NULL) {
		_err("Super Class '");
		_err(super_name);
		_err("' not found!\n");
		return;
	}

	var_t* protoV = get_prototype(n->var);
	if(protoV != NULL)
		var_add(clsProto, SUPER, protoV);
}

/** create object by classname or function */
var_t* new_obj(vm_t* vm, const char* name, int arg_num) {
	var_t* obj = NULL;
	node_t* n = vm_load_node(vm, name, false); //load class;

	if(n == NULL || n->var->type != V_OBJECT) {
		_err("Error: There is no class: '");
		_err(name);
		_err("'!\n");
		return NULL;
	}

	var_t* protoV = get_prototype(n->var);
	var_t* constructor = NULL;
	obj = var_new_obj(NULL, NULL);

	if(n->var->is_func) { // new object built by function call
		constructor = n->var;
	}
	else {
		constructor = var_find_var(protoV, CONSTRUCTOR);
	}

	if(constructor != NULL) {
		func_call(vm, obj, constructor, arg_num);
		obj = vm_pop2(vm);
		var_unref(obj, false);
	}
	var_add(obj, PROTOTYPE, protoV);
	return obj;
}

int parse_func_name(const char* full, str_t* name) {
	const char* pos = strchr(full, '$');
	int args_num = 0;
	if(pos != NULL) {
		args_num = atoi(pos+1);
		if(name != NULL)
			str_ncpy(name, full, pos-full);	
	}
	else {
		if(name != NULL)
			str_cpy(name, full);
	}
	return args_num;
}

/** create object and try constructor */
bool do_new(vm_t* vm, const char* full) {
	str_t* name = str_new("");
	int arg_num = parse_func_name(full, name);

	var_t* obj = new_obj(vm, name->cstr, arg_num);
	str_free(name);

	if(obj == NULL)
		return false;
	vm_push(vm, obj);
	return true;
}

var_t* call_m_func(vm_t* vm, var_t* obj, var_t* func, var_t* args) {
	//push args to stack.
	int arg_num = 0;
	if(args != NULL) {
		arg_num = args->children.size;
		int i;
		for(i=0; i<arg_num; i++) {
			var_t* v = ((node_t*)args->children.items[i])->var;
			vm_push(vm, v);
		}
	}

	func_call(vm, obj, func, arg_num);
	return vm_pop2(vm);
}

var_t* call_m_func_by_name(vm_t* vm, var_t* obj, const char* func_name, var_t* args) {
	node_t* func = find_member(obj, func_name);
	if(func == NULL || func->var->is_func == 0) {
		_err("Interrupt function '");
		_err(func_name);
		_err("' not defined!\n");
		return NULL;
	}
	return call_m_func(vm, obj, func->var, args);
}

#ifdef MARIO_THREAD
/**
interrupter
*/

#define MAX_ISIGNAL 128

static bool interrupt_raw(vm_t* vm, var_t* obj, const char* func_name, var_t* func, var_t* args) {
	while(vm->interrupted) { } // can not interrupt another interrupter.

	pthread_mutex_lock(&vm->interrupt_lock);
	if(vm->isignal_num >= MAX_ISIGNAL) {
		_err("Too many interrupt signals!\n");
		if(args != NULL)
			var_unref(args, true);
		pthread_mutex_unlock(&vm->interrupt_lock);
		return false;
	}

	isignal_t* is = (isignal_t*)_malloc(sizeof(isignal_t));
	if(is == NULL) {
		_err("Interrupt signal input error!\n");
		if(args != NULL)
			var_unref(args, true);
		pthread_mutex_unlock(&vm->interrupt_lock);
		return false;
	}

	is->next = NULL;
	is->prev = NULL;
	is->obj = var_ref(obj);

	if(func != NULL)
		is->handle_func = var_ref(func);
	else
		is->handle_func = NULL;

	if(func_name != NULL && func_name[0] != 0) 
		is->handle_func_name = str_new(func_name);
	else
		is->handle_func_name = NULL;

	if(args != NULL)
		is->args = var_ref(args);
	else
		is->args = NULL;

	if(vm->isignal_tail == NULL) {
		vm->isignal_head = is;
		vm->isignal_tail = is;
	}
	else {
		vm->isignal_tail->next = is;
		is->prev = vm->isignal_tail;
		vm->isignal_tail = is;
	}

	vm->isignal_num++;
	pthread_mutex_unlock(&vm->interrupt_lock);
	return true;
}

bool interrupt_by_name(vm_t* vm, var_t* obj, const char* func_name, var_t* args) {
	return interrupt_raw(vm, obj, func_name, NULL, args);
}

bool interrupt(vm_t* vm, var_t* obj, var_t* func, var_t* args) {
	return interrupt_raw(vm, obj, NULL, func, args);
}

void try_interrupter(vm_t* vm) {
	if(vm->isignal_head == NULL || vm->interrupted) {
		return;
	}

	pthread_mutex_lock(&vm->interrupt_lock);
	vm->interrupted = true;

	isignal_t* sig = vm->isignal_head;
		
	var_t* func = NULL;

	while(sig != NULL) {
		if(sig->handle_func != NULL) {
			func = sig->handle_func;
			break;
		}
		else if(sig->handle_func_name != NULL) {
			//if func undefined yet, keep it and try next sig
			node_t* func_node = find_member(sig->obj, sig->handle_func_name->cstr);
			if(func_node != NULL && func_node->var->is_func) { 
				func = var_ref(func_node->var);
				break;
			}
		}
		sig = sig->next;
	}

	if(func == NULL || sig == NULL) {
		vm->interrupted = false;
		pthread_mutex_unlock(&vm->interrupt_lock);
		return;
	}

	//pop this sig from queue.
	if(sig->prev == NULL) 
		vm->isignal_head = sig->next;
	else 
		sig->prev->next = sig->next;

	if(sig->next == NULL)
		vm->isignal_tail = sig->prev;
	else
		sig->next->prev = sig->prev;

	m_array_t* sc = vm->scopes;
	func_t* f = var_get_func(func);
	if(f != NULL && f->scopes != NULL)
		vm->scopes = f->scopes;
	var_t* ret = call_m_func(vm, sig->obj, func, sig->args);
	vm->scopes = sc;

	if(ret != NULL)
		var_unref(ret, true);

	var_unref(sig->obj, true);
	var_unref(func, true);
	if(sig->handle_func_name != NULL)
		str_free(sig->handle_func_name);
	if(sig->args != NULL)
		var_unref(sig->args, true);
	_free(sig);
	vm->isignal_num--;
	vm->interrupted = false;
	pthread_mutex_unlock(&vm->interrupt_lock);
}

#else

bool interrupt(vm_t* vm, var_t* obj, var_t* func, var_t* args) {
	return false;
}	

bool interrupt_by_name(vm_t* vm, var_t* obj, const char* func_name, var_t* args) {
	return false;
}	

#endif

/*****************/

node_t* vm_new_class(vm_t* vm, const char* cls) {
	node_t* cls_node = vm_load_node(vm, cls, true);
	cls_node->var->type = V_OBJECT;

	if(get_prototype(cls_node->var) == NULL) {
		if(strcmp(cls, "Object") == 0) {
			vm->var_Object = cls_node->var;
			add_prototype(vm, vm->var_Object, NULL);
		}
		else {
			add_prototype(vm, cls_node->var, vm->var_Object);
		}
	}

	return cls_node;
}

void vm_terminate(vm_t* vm) {
	while(true) { //clear stack
		if(!vm_pop(vm))
			break;
	}

	while(true) { //clear scopes
		scope_t* sc = vm_get_scope(vm);
		if(sc == NULL) break;
		vm_pop_scope(vm);
	}
	vm->pc = vm->bc.cindex;
}

void vm_run(vm_t* vm) {
	//int32_t scDeep = vm->scopes.size;
	register PC code_size = vm->bc.cindex;
	register PC* code = vm->bc.code_buf;

	do {
		register PC ins = code[vm->pc++];
		register opr_code_t instr = OP(ins);
		register uint32_t offset = OFF(ins);

		if(instr == INSTR_END)
			break;
		
		switch(instr) {
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
			case INSTR_NJMPB: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					if(v->type == V_UNDEF ||
							v->value == NULL ||
							*(int*)(v->value) == 0) {
						if(instr == INSTR_NJMP) 
							vm->pc = vm->pc + offset - 1;
						else
							vm->pc = vm->pc - offset - 1;
					}
					var_unref(v, true);
				}
				break;
			}
			case INSTR_LOAD: 
			case INSTR_LOADO: 
			{
				bool loaded = false;
				if(offset == vm->this_strIndex) {
					var_t* thisV = vm_this_in_scopes(vm);
					if(thisV != NULL) {
						vm_push(vm, thisV);	
						loaded = true;	
					}
				}
				if(!loaded) {
					const char* s = bc_getstr(&vm->bc, offset);
					node_t* n = NULL;
					if(instr == INSTR_LOAD) {
						n = vm_load_node(vm, s, true); //load variable, create if not exist.
						vm_push_node(vm, n);
					}
					else { // LOAD obj
						n = vm_load_node(vm, s, false); //load variable, warning if not exist.
						if(n != NULL) 
							vm_push_node(vm, n);
						else {
							_err("Error: object or class '");
							_err(s);
							_err("' undefined!\n");
							vm_terminate(vm);
						}
					}
				}
				break;
			}
			case INSTR_LES: 
			case INSTR_EQ: 
			case INSTR_NEQ: 
			case INSTR_TEQ:
			case INSTR_NTEQ:
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
			case INSTR_NIL: 
			{	break; }
			case INSTR_BLOCK: 
			case INSTR_LOOP: 
			case INSTR_TRY: 
			{
				scope_t* sc = NULL;
				sc = scope_new(var_new_block());
				if(instr == INSTR_LOOP) {
					sc->is_loop = true;
					sc->pc_start = vm->pc+1;
					sc->pc = vm->pc+2;
				}
				else if(instr == INSTR_TRY) {
					sc->is_try = true;
					sc->pc = vm->pc+1;
				}
				vm_push_scope(vm, sc);
				break;
			}
			case INSTR_BLOCK_END: 
			case INSTR_LOOP_END: 
			case INSTR_TRY_END: 
			{
				vm_pop_scope(vm);
				break;
			}
			case INSTR_BREAK: 
			{
				while(true) {
					scope_t* sc = vm_get_scope(vm);
					if(sc == NULL) {
						_err("Error: 'break' not in any loop!\n");
						vm_terminate(vm);
						break;
					}
					if(sc->is_loop) {
						vm->pc = sc->pc;
						break;
					}
					vm_pop_scope(vm);
				}
				break;
			}
			case INSTR_CONTINUE:
			{
				while(true) {
					scope_t* sc = vm_get_scope(vm);
					if(sc == NULL) {
						_err("Error: 'continue' not in any loop!\n");
						vm_terminate(vm);
						break;
					}
					if(sc->is_loop) {
						vm->pc = sc->pc_start;
						break;
					}
					vm_pop_scope(vm);
				}
				break;
			}

			#ifdef MARIO_CACHE
			case INSTR_CACHE: 
			{	
				var_t* v = vm->var_cache[offset];
				vm_push(vm, v);
				break;
			}
			#endif
			case INSTR_TRUE: 
			{
				vm_push(vm, vm->var_true);
				break;
			}
			case INSTR_FALSE: 
			{
				vm_push(vm, vm->var_false);
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
					bool i = false;
					if(v->type == V_UNDEF || *(int*)v->value == 0)
						i = true;
					var_unref(v, true);
					vm_push(vm, i ? vm->var_true:vm->var_false);
				}
				break;
			}
			case INSTR_AAND: 
			case INSTR_OOR: 
			{
				var_t* v2 = vm_pop2(vm);
				var_t* v1 = vm_pop2(vm);
				if(v1 != NULL && v2 != NULL) {
					bool r = false;
					int i1 = *(int*)v1->value;
					int i2 = *(int*)v2->value;

					if(instr == INSTR_AAND)
						r = (i1 != 0) && (i2 != 0);
					else
						r = (i1 != 0) || (i2 != 0);
					vm_push(vm, r ? vm->var_true:vm->var_false);

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
					if(i != NULL) {
						(*i)--;
						if((ins & INSTR_OPT_CACHE) == 0) {
							if(OP(code[vm->pc]) != INSTR_POP) { 
								vm_push(vm, v);
							}
							else { code[vm->pc] = INSTR_NIL; code[vm->pc-1] |= INSTR_OPT_CACHE; }
						}
					}
					else {
						vm_push(vm, v);
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
					if(i != NULL) {
						if((ins & INSTR_OPT_CACHE) == 0) {
							var_t* v2 = var_new_int(*i);
							if(OP(code[vm->pc]) != INSTR_POP) {
								vm_push(vm, v2);
							}
							else { 
								code[vm->pc] = INSTR_NIL; 
								code[vm->pc-1] |= INSTR_OPT_CACHE;
								var_unref(v2, true);
							}
						}
						(*i)--;
					}
					else {
						vm_push(vm, v);
					}
					var_unref(v, true);
				}
				break;
			}
			case INSTR_PPLUS_PRE: 
			{
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					int *i = (int*)v->value;
					if(i != NULL) {
						(*i)++;

						if((ins & INSTR_OPT_CACHE) == 0) {
							if(OP(code[vm->pc]) != INSTR_POP) { 
								vm_push(vm, v);
							}
							else { code[vm->pc] = INSTR_NIL; code[vm->pc-1] |= INSTR_OPT_CACHE; }
						}
					}
					else {
						vm_push(vm, v);
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
					if(i != NULL) {
						if((ins & INSTR_OPT_CACHE) == 0) {
							var_t* v2 = var_new_int(*i);
							if(OP(code[vm->pc]) != INSTR_POP) {
								vm_push(vm, v2);
							}
							else { 
								code[vm->pc] = INSTR_NIL;
								code[vm->pc-1] |= INSTR_OPT_CACHE; 
								var_unref(v2, true);
							}
						}

						(*i)++;
					}
					else {
						vm_push(vm, v);
					}
					var_unref(v, true);
				}
				break;
			}
			case INSTR_RETURN:  //return without value
			case INSTR_RETURNV: 
			{ //return with value
				if(instr == INSTR_RETURN) {//return without value, push "this" to stack
					var_t* thisV = vm_this_in_scopes(vm);
					if(thisV != NULL)
						vm_push(vm, thisV);
					else
						vm_push(vm, var_new());
				}

				while(true) {
					scope_t* sc = vm_get_scope(vm);
					if(sc == NULL) break;
					if(sc->is_func) {
						vm->pc = sc->pc;
						vm_pop_scope(vm);
						break;
					}
					vm_pop_scope(vm);
				}
				return;
			}
			case INSTR_VAR:
			{
				const char* s = bc_getstr(&vm->bc, offset);
				node_t *node = vm_find(vm, s);
				if(node != NULL) { //find just in current scope
					_err("Warning: '");
					_err(s);
					_err("' has already existed!\n");
				}
				else {
					var_t* v = vm_get_scope_var(vm, true);
					if(v != NULL) {
						node = var_add(v, s, NULL);
					}
				}
				break;
			}
			case INSTR_LET:
			case INSTR_CONST: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				var_t* v = vm_get_scope_var(vm, false);
				node_t *node = var_find(v, s);
				if(node != NULL) { //find just in current scope
					_err("Error: let '");
					_err(s);
					_err("' has already existed!\n");
					vm_terminate(vm);
				}
				else {
					node = var_add(v, s, NULL);
					if(node != NULL && instr == INSTR_CONST)
						node->be_const = true;
				}
				break;
			}
			case INSTR_INT:
			{
				var_t* v = var_new_int((int)code[vm->pc++]);
				#ifdef MARIO_CACHE
				if(try_cache(vm, &code[vm->pc-2], v))
					code[vm->pc-1] = INSTR_NIL;
				#endif
				vm_push(vm, v);
				break;
			}
			case INSTR_INT_S:
			{
				var_t* v = var_new_int(offset);
				#ifdef MARIO_CACHE
				try_cache(vm, &code[vm->pc-1], v);
				#endif
				vm_push(vm, v);
				break;
			}
			case INSTR_FLOAT: 
			{
				var_t* v = var_new_float(*(float*)(&code[vm->pc++]));
				#ifdef MARIO_CACHE
				if(try_cache(vm, &code[vm->pc-2], v))
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
				try_cache(vm, &code[vm->pc-1], v);
				#endif
				vm_push(vm, v);
				break;
			}
			case INSTR_ASIGN: 
			{
				var_t* v = vm_pop2(vm);
				node_t* n = vm_pop2node(vm);
				if(v != NULL && n != NULL) {
					bool modi = (!n->be_const || n->var->type == V_UNDEF);
					var_unref(n->var, true);
					if(modi) 
						node_replace(n, v);
					else {
						_err("Can not change a const variable: '");
						_err(n->name);
						_err("'!\n");
					}
					var_unref(v, true);

					if((ins & INSTR_OPT_CACHE) == 0) {
						if(OP(code[vm->pc]) != INSTR_POP) {
							vm_push(vm, n->var);
						}
						else { code[vm->pc] = INSTR_NIL; code[vm->pc-1] |= INSTR_OPT_CACHE; }
					}
				}
				break;
			}
			case INSTR_GET: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					var_build_basic_prototype(vm, v);
					do_get(vm, v, s);
					var_unref(v, true);
				}
				else {
					vm_push(vm, var_new());
					_err("Error: can not find member '");
					_err(s);
					_err("'!\n");
				}
				break;
			}
			case INSTR_NEW: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				if(!do_new(vm, s)) 
					vm_terminate(vm);
				break;
			}
			case INSTR_CALL: 
			case INSTR_CALLO: 
			{
				var_t* func = NULL;
				var_t* obj = NULL;
				bool unrefObj = false;
				const char* s = bc_getstr(&vm->bc, offset);
				str_t* name = str_new("");
				int arg_num = parse_func_name(s, name);
				
				if(instr == INSTR_CALLO) {
					obj = vm_stack_pick(vm, arg_num+1);
					var_build_basic_prototype(vm, obj);
					unrefObj = true;
				}
				else {
					obj = vm_this_in_scopes(vm);
				}

				func = find_func(vm, obj, name->cstr);
				if(func != NULL && !func->is_func) { //constructor like super()
					var_t* constr = var_find_var(func, CONSTRUCTOR);	
					if(constr == NULL) {
						var_t* protoV = get_prototype(func);
						if(protoV != NULL) 
							func = var_find_var(protoV, CONSTRUCTOR);	
						else
							func = NULL;
					}
					else {
						func = constr;
					}
				}

				if(func != NULL) {
					func_call(vm, obj, func, arg_num);
				}
				else {
					while(arg_num > 0) {
						vm_pop(vm);
						arg_num--;
					}
					vm_push(vm, var_new());
					_err("Error: can not find function '");
					_err(s);
					_err("'!\n");
				}
				str_free(name);

				if(unrefObj && obj != NULL)
					var_unref(obj, true);

				//check and do interrupter.
				#ifdef MARIO_THREAD
				try_interrupter(vm);
				#endif
				break;
			}
			case INSTR_MEMBER: 
			case INSTR_MEMBERN: 
			{
				const char* s = (instr == INSTR_MEMBER ? "" :  bc_getstr(&vm->bc, offset));
				var_t* v = vm_pop2(vm);
				if(v != NULL) {
					var_t *var = vm_get_scope_var(vm, true);
					if(var->is_array)
						var = get_obj(var, "_ARRAY_");

					if(var != NULL) {
						if(v->is_func) {
							func_t* func = (func_t*)v->value;
							func->owner = var;
						}
						var_add(var, s, v);
					}
					var_unref(v, true);
				}
				break;
			}

			case INSTR_FUNC: 
			case INSTR_FUNC_STC: 
			case INSTR_FUNC_GET: 
			case INSTR_FUNC_SET: 
			{
				var_t* v = func_def(vm, 
						(instr == INSTR_FUNC ? true:false),
						(instr == INSTR_FUNC_STC ? true:false));
				if(v != NULL)
					vm_push(vm, v);
				break;
			}
			case INSTR_OBJ:
			case INSTR_ARRAY: 
			{
				var_t* obj;
				if(instr == INSTR_OBJ) {
					obj = var_new_obj(NULL, NULL);
					var_add(obj, PROTOTYPE, get_prototype(vm->var_Object));
				}
				else
					obj = var_new_array();
				scope_t* sc = scope_new(obj);
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
					node_t* n = var_array_get(v1, at);
					if(n != NULL) 
						vm_push_node(vm, n);
					var_unref(v1, true);
				}
				break;
			}
			case INSTR_CLASS: 
			{
				const char* s =  bc_getstr(&vm->bc, offset);
				node_t* n = vm_new_class(vm, s);
				var_t* protoV = get_prototype(n->var);
				//read extends
				ins = code[vm->pc];
				instr = OP(ins);
				if(instr == INSTR_EXTENDS) {
					vm->pc++;
					offset = OFF(ins);
					s =  bc_getstr(&vm->bc, offset);
					doExtends(vm, protoV, s);
				}

				scope_t* sc = scope_new(protoV);
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
			case INSTR_TYPEOF: 
			{
				var_t* var = vm_pop2(vm);
				var_t* v = var_new_str(get_typeof(var));
				vm_push(vm, v);
				break;
			}
			case INSTR_THROW: 
			{
				while(true) {
					scope_t* sc = vm_get_scope(vm);
					if(sc == NULL) {
						_err("Error: 'throw' not in any try...catch!\n");
						vm_terminate(vm);
						break;
					}
					if(sc->is_try) {
						vm->pc = sc->pc;
						break;
					}
					vm_pop_scope(vm);
				}
				break;
			}
			case INSTR_CATCH: 
			{
				const char* s = bc_getstr(&vm->bc, offset);
				var_t* v = vm_pop2(vm);
				var_t* sc_var = vm_get_scope_var(vm, false);
				var_add(sc_var, s, v);
				var_unref(v, true);
				break;
			}

		}
	}
	while(vm->pc < code_size);
}

bool vm_load(vm_t* vm, const char* s) {
	if(vm->compiler == NULL)
		return false;

	/*
	if(vm->bc.cindex > 0) {
		vm->bc.cindex--;
	}
	*/
	vm->pc = vm->bc.cindex;
	return vm->compiler(&vm->bc, s);
}

bool vm_load_run(vm_t* vm, const char* s) {
	bool ret = false;
	if(vm_load(vm, s)) {
		vm_run(vm);
		ret = true;
	}
	return ret;
}

bool vm_load_run_native(vm_t* vm, const char* s) {
	bool ret = false;
	PC old = vm->pc;

	if(vm_load(vm, s)) {
		vm_run(vm);
		ret = true;
	}

	vm->pc = old;
	return ret;
}

typedef struct st_native_init {
	void (*func)(void*);
	void *data;
} native_init_t;


void vm_dump(vm_t* vm) {
#ifdef MARIO_DEBUG
	bc_dump(&vm->bc);
#endif
}

void vm_close(vm_t* vm) {
	vm->terminated = true;
	int i;
	for(i=0; i<vm->close_natives.size; i++) {
		native_init_t* it = (native_init_t*)array_get(&vm->close_natives, i);
		it->func(it->data);
	}
	array_clean(&vm->close_natives, NULL);

	var_unref(vm->root, true);
	var_unref(vm->var_true, true);
	var_unref(vm->var_false, true);


	#ifdef MARIO_THREAD
	pthread_mutex_destroy(&vm->interrupt_lock);
	#endif

	#ifdef MARIO_CACHE
	var_cache_free(vm);
	#endif

	array_free(vm->scopes, scope_free);
	array_clean(&vm->init_natives, NULL);


	bc_release(&vm->bc);
	vm->stack_top = 0;

	if(vm->on_close != NULL)
		vm->on_close(vm);
	_free(vm);
}	

/** native extended functions.-----------------------------*/

void vm_reg_init(vm_t* vm, void (*func)(void*), void* data) {
	native_init_t* it = (native_init_t*)_malloc(sizeof(native_init_t));
	it->func = func;
	it->data = data;
	array_add(&vm->init_natives, it);
}

void vm_reg_close(vm_t* vm, void (*func)(void*), void* data) {
	native_init_t* it = (native_init_t*)_malloc(sizeof(native_init_t));
	it->func = func;
	it->data = data;
	array_add(&vm->close_natives, it);
}

node_t* vm_reg_var(vm_t* vm, const char* cls, const char* name, var_t* var, bool be_const) {
	var_t* cls_var = vm->root;
	if(cls[0] != 0) {
		node_t* clsnode = vm_new_class(vm, cls);
		cls_var = get_prototype(clsnode->var);
		//cls_var = clsnode->var;
	}

	node_t* node = var_add(cls_var, name, var);
	node->be_const = be_const;
	return node;
}

node_t* vm_reg_native(vm_t* vm, const char* cls, const char* decl, native_func_t native, void* data) {
	var_t* cls_var = vm->root;
	if(cls[0] != 0) {
		node_t* cls_node = vm_new_class(vm, cls);
		cls_var = get_prototype(cls_node->var);
		//cls_var = cls_node->var;
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

	var_t* var = var_new_func(vm, func);
	node_t* node = var_add(cls_var, name->cstr, var);
	str_free(name);

	return node;
}

node_t* vm_reg_static(vm_t* vm, const char* cls, const char* decl, native_func_t native, void* data) {
	node_t* n = vm_reg_native(vm, cls, decl, native, data);
	func_t* func = var_get_func(n->var);
	func->is_static = true;
	return n;
}

const char* get_str(var_t* var, const char* name) {
	var_t* v = get_obj(var, name);
	return v == NULL ? "" : var_get_str(v);
}

int get_int(var_t* var, const char* name) {
	var_t* v = get_obj(var, name);
	return v == NULL ? 0 : var_get_int(v);
}

bool get_bool(var_t* var, const char* name) {
	var_t* v = get_obj(var, name);
	return v == NULL ? 0 : var_get_bool(v);
}

float get_float(var_t* var, const char* name) {
	var_t* v = get_obj(var, name);
	return v == NULL ? 0 : var_get_float(v);
}

var_t* get_obj(var_t* var, const char* name) {
	//if(strcmp(name, THIS) == 0)
	//	return var;
	node_t* n = var_find(var, name);
	if(n == NULL)
		return NULL;
	return n->var;
}

void* get_raw(var_t* var, const char* name) {
	var_t* v = get_obj(var, name);
	if(v == NULL)
		return NULL;
	return v->value;
}

var_t* get_obj_member(var_t* env, const char* name) {
	var_t* obj = get_obj(env, THIS);
	if(obj == NULL)
		return NULL;
	return var_find_var(obj, name);
}

var_t* set_obj_member(var_t* env, const char* name, var_t* var) {
	var_t* obj = get_obj(env, THIS);
	if(obj == NULL)
		return NULL;
	var_add(obj, name, var);
	return var;
}

var_t* native_debug(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* v = var_find_var(env, "v");
	str_t* s = str_new("");
	var_to_str(v, s);
	str_add(s, '\n');
	_out_func(s->cstr);
	str_free(s);
	return NULL;
}

/**yield */
var_t* native_yield(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data; (void)env;
	return NULL;
}

vm_t* vm_from(vm_t* vm) {
	vm_t* ret = vm_new(vm->compiler);
  vm_init(ret, vm->on_init, vm->on_close);
	return ret;
}

vm_t* vm_new(bool compiler(bytecode_t *bc, const char* input)) {
	vm_t* vm = (vm_t*)_malloc(sizeof(vm_t));
	memset(vm, 0, sizeof(vm_t));

	vm->compiler = compiler;
	vm->terminated = false;
	vm->pc = 0;
	vm->this_strIndex = 0;
	vm->stack_top = 0;

	bc_init(&vm->bc);
	vm->this_strIndex = bc_getstrindex(&vm->bc, THIS);

	vm->scopes = array_new();

	vm->var_true = var_new_bool(true);
	var_ref(vm->var_true);
	vm->var_false = var_new_bool(false);
	var_ref(vm->var_false);

	#ifdef MARIO_CACHE
	var_cache_init(vm);
	#endif

	#ifdef MARIO_THREAD
	pthread_mutex_init(&vm->interrupt_lock, NULL);

	vm->isignal_head = NULL;
	vm->isignal_tail = NULL;
	vm->isignal_num = 0;
	vm->interrupted = false;

	#endif

	array_init(&vm->close_natives);	
	array_init(&vm->init_natives);	
	
	vm->root = var_new_obj(NULL, NULL);
	var_ref(vm->root);

	vm_new_class(vm, "Object");

	vm_reg_native(vm, "", "debug(v)", native_debug, NULL);
	vm_reg_native(vm, "", "yield()", native_yield, NULL);

	return vm;
}

void vm_init(vm_t* vm,
		void (*on_init)(struct st_vm* vm),
		void (*on_close)(struct st_vm* vm)) {
	vm->on_init = on_init;
	vm->on_close = on_close;
	
	if(vm->on_init != NULL)
		vm->on_init(vm);

	int i;
	for(i=0; i<vm->init_natives.size; i++) {
		native_init_t* it = (native_init_t*)array_get(&vm->init_natives, i);
		it->func(it->data);
	}

	vm_load_basic_classes(vm);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

