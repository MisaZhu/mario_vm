#ifndef ARDUINO_BASIC_NATIVE_H
#define ARDUINO_BASIC_NATIVE_H

#include "mario.h"

var_t* native_serial_print(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "str");
	const char* s = n == NULL ? "" : var_get_str(n->var);
	Serial.print(s);
	return NULL;
}

var_t* native_serial_write(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "bytes");
	if(n == NULL || n->var == NULL || n->var->size == 0)
		return NULL;
	var_t* bytes = n->var;

  int bytesSize = bytes->size;
  if(bytes->type == V_STRING)
    bytesSize--;

	n = var_find(env, "size");
	int size = n == NULL ? 0 : var_get_int(n->var);
	if(size > bytesSize)
		size = bytesSize;

	int res = Serial.write((const uint8_t*)bytes->value, size);
	return var_new_int(res);
}

var_t* native_serial_begin(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "bps");
	int bps = n == NULL ? 9600 : var_get_int(n->var);

	Serial.begin(bps);
	return NULL;
}

var_t*  native_pinMode(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "pin");
	int pin = n == NULL ? 0 : var_get_int(n->var);

	n = var_find(env, "type");
	int type = n == NULL ? 0 : var_get_int(n->var);

	pinMode(pin, type);
	return NULL;
}

var_t*  native_digitalWrite(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "pin");
	int pin = n == NULL ? 0 : var_get_int(n->var);

	n = var_find(env, "type");
	int type = n == NULL ? 0 : var_get_int(n->var);

	digitalWrite(pin, type);
	return NULL;
}

var_t* native_delay(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "msec");
	int msec = n == NULL ? 0 : var_get_int(n->var);

	delay(msec);
	return NULL;
}

#define CLS_SERIAL "Serial"

void reg_native_arduino_basic(vm_t* vm) {
	vm_reg_var(vm, "", "LED_BUILTIN", var_new_int(LED_BUILTIN));
	vm_reg_var(vm, "", "OUTPUT", var_new_int(OUTPUT));
	vm_reg_var(vm, "", "LOW", var_new_int(LOW));
	vm_reg_var(vm, "", "HIGH", var_new_int(HIGH));

	vm_reg_native(vm, "", "print(str)", native_serial_print, NULL);
	vm_reg_native(vm, "", "delay(msec)", native_delay, NULL);
	vm_reg_native(vm, "", "pinMode(pin, type)", native_pinMode, NULL);
	vm_reg_native(vm, "", "digitalWrite(pin, type)", native_digitalWrite, NULL);

	vm_reg_native(vm, CLS_SERIAL, "print(str)", native_serial_print, NULL);
	vm_reg_native(vm, CLS_SERIAL, "begin(bps)", native_serial_begin, NULL);
	vm_reg_native(vm, CLS_SERIAL, "write(bytes, size)", native_serial_write, NULL);
}

#endif

