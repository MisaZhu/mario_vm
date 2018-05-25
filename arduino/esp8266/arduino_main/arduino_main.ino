#include "mario.h"

vm_t _vm;

void dump(const char* s) {
  Serial.print(s);
}

var_t* native_print(vm_t* vm, var_t* env, void* data) {
  node_t* n = var_get(env, 0);
  if(n->var == NULL || n->var->value == NULL || n->var->type != V_STRING)
    return NULL;

  dump((const char*)n->var->value);
  return NULL;
}


void setup() {  
  Serial.begin(9600);
  delay(200);
  
  const char* s = "print('hello, world');";
  
  vm_init(&_vm);
  vm_reg_native(&_vm, "print", 1, native_print);
  
  vm_load(&_vm, s, dump);
  vm_run(&_vm);
  vm_close(&_vm);
}

void loop() {
  delay(1000);
}
