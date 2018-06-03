#ifdef __cplusplus
extern "C" {
#endif

#include "native_basic.h"

/** Bytes */

var_t* native_BytesConstructor(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	node_t* n = var_find(env, "size");
	int sz = n == NULL ? 0 : var_get_int(n->var);
	if(sz <= 0)
		return NULL;

	var_t* thisV = get_env_this(env);
	if(thisV != NULL) {
		thisV->value = _malloc(sz);
		thisV->size = sz;
		memset(thisV->value, 0, sz);
	}
	return thisV;
}

var_t* native_BytesSize(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* thisV = get_env_this(env);
	int sz = thisV == NULL ? 0 : thisV->size;
	return var_new_int(sz);
}

var_t* native_BytesToString(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* thisV = get_env_this(env);
	const char* s = thisV == NULL ? "" : (const char*)thisV->value;
	return var_new_str(s);
}

var_t* native_BytesAt(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "index");
	int index = n == NULL ? 0 : var_get_int(n->var);

	var_t* thisV = get_env_this(env);
	int sz = thisV == NULL ? 0 : thisV->size;

	int i = 0;
	if(sz > index)
		i = ((char*)thisV->value)[index];
		
	return var_new_int(i);
}

var_t* native_BytesSet(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "index");
	int index = n == NULL ? 0 : var_get_int(n->var);
	n = var_find(env, "i");
	int i = n == NULL ? 0 : var_get_int(n->var);

	var_t* thisV = get_env_this(env);
	int sz = thisV == NULL ? 0 : thisV->size;

	if(sz > index)
		((char*)thisV->value)[index] = i;
		
	return NULL;
}

var_t* native_BytesFromString(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "str");
	const char* s = n == NULL ? "" : var_get_str(n->var);

	var_t* thisV = get_env_this(env);
	if(thisV != NULL) {
		thisV->size = strlen(s) + 1;
		thisV->value = _realloc(thisV->value, thisV->size);
		memcpy(thisV->value, s, thisV->size);
	}
		
	return NULL;
}

/** String */
var_t* native_StringConstructor(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "str");
	const char* s = n == NULL ? "" : var_get_str(n->var);

	var_t* thisV = get_env_this(env);
	thisV->size = strlen(s) + 1;
	thisV->value = _malloc(thisV->size);
	memcpy(thisV->value, s, thisV->size);
	return thisV;
}

var_t* native_StringLength(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* thisV = get_env_this(env);
	int len = 0;
	if(thisV != NULL && thisV->value != NULL)
		len = strlen((const char*)thisV->value);
	return var_new_int(len);
}

var_t* native_StringToString(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* thisV = get_env_this(env);
	const char* s = thisV == NULL ? "" : (const char*)thisV->value;
	return var_new_str(s);
}

void reg_native_basic(vm_t* vm) {
	vm_reg_native(vm, "Bytes", "constructor(size)", native_BytesConstructor, NULL); 
	vm_reg_native(vm, "Bytes", "size()", native_BytesSize, NULL); 
	vm_reg_native(vm, "Bytes", "toString()", native_BytesToString, NULL); 
	vm_reg_native(vm, "Bytes", "fromString(str)", native_BytesFromString, NULL); 
	vm_reg_native(vm, "Bytes", "set(index, i)", native_BytesSet, NULL); 
	vm_reg_native(vm, "Bytes", "at(index)", native_BytesAt, NULL); 

	vm_reg_native(vm, "String", "constructor(str)", native_StringConstructor, NULL); 
	vm_reg_native(vm, "String", "length()", native_StringLength, NULL); 
	vm_reg_native(vm, "String", "toString()", native_StringToString, NULL); 
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
