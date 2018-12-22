# Mario VM
very tiny bytecode vm engine, None 3rd libs relied, so can be used on most of embedded systems.

You have to implement "bool compile(bytecode_t \*bc, const char\* input)" function for your own language(check lang/demo/compiler.c). "const char \*_mario_lang"(declared in mario_vm.h) must be assigned as well.

"runtime", a project based on MarioVM with native classes dynamic libaray loading.
