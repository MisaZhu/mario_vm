#ifdef __cplusplus
extern "C" {
#endif

#include "native_basic.h"

/** Bytes */

var_t* native_BytesConstructor(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* thisV = get_obj(env, THIS);
	int sz = get_int(env, "size");
	if(sz <= 0)
		return thisV;

	if(thisV != NULL) {
		thisV->value = _malloc(sz+1);
		memset(thisV->value, 0, sz+1);
		thisV->size = sz;
	}
	return thisV;
}

var_t* native_BytesSize(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* thisV = get_obj(env, THIS);
	int sz = thisV == NULL ? 0 : thisV->size;
	return var_new_int(sz);
}

var_t* native_BytesToString(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* thisV = get_obj(env, THIS);
	const char* s = thisV == NULL ? "" : (const char*)thisV->value;
	return var_new_str(s);
}

var_t* native_BytesAt(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	int index = get_int(env, "index");

	var_t* thisV = get_obj(env, THIS);
	int sz = thisV == NULL ? 0 : thisV->size;

	int i = 0;
	if(sz > index)
		i = ((char*)thisV->value)[index];
		
	return var_new_int(i);
}

var_t* native_BytesSet(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	int index = get_int(env, "index");
	int i = get_int(env, "i");

	var_t* thisV = get_obj(env, THIS);
	int sz = thisV == NULL ? 0 : thisV->size;

	if(sz > index)
		((char*)thisV->value)[index] = i;
		
	return NULL;
}

var_t* native_BytesFromString(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	const char* s = get_str(env, "str");

	var_t* thisV = get_obj(env, THIS);
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

	const char* s = get_str(env, "str");

	var_t* thisV = get_obj(env, THIS);
	thisV->size = strlen(s) + 1;
	thisV->value = _malloc(thisV->size);
	memcpy(thisV->value, s, thisV->size);
	return thisV;
}

var_t* native_StringLength(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* thisV = get_obj(env, THIS);
	int len = 0;
	if(thisV != NULL && thisV->value != NULL)
		len = strlen((const char*)thisV->value);
	return var_new_int(len);
}

var_t* native_StringToString(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* thisV = get_obj(env, THIS);
	const char* s = thisV == NULL ? "" : (const char*)thisV->value;
	return var_new_str(s);
}

/**JSON functions */
var_t* native_json_stringify(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "var");
	str_t* s = str_new("");
	if(n != NULL)
		var_to_json_str(n->var, s, 0);

	var_t* var = var_new_str(s->cstr);
	str_free(s);
	return var;
}

var_t* native_json_parse(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	const char* s = get_str(env, "str");
	return json_parse(s);
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

	vm_reg_native(vm, "JSON", "stringify(var)", native_json_stringify, NULL);
	vm_reg_native(vm, "JSON", "parse(str)", native_json_parse, NULL);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
