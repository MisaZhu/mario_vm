#include "mario.h"

vm_t _vm;

void dump(const char* s) {
  Serial.print(s);
}

void setup() {  
  Serial.begin(9600);
  delay(200);
  
  const char* s = "var abc = 0;";
  vm_init(&_vm);
  vm_load(&_vm, s, dump);
  vm_run(&_vm);
  vm_close(&_vm);
}

void loop() {
  delay(1000);
}
