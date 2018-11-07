# Mario VM
very tiny bytecode vm engine, None 3rd libs relied, so can be used on most of embedded systems. You have to implement "bool compile(bytecode_t \*bc, const char\* input)" function for your own language(check demo/demo_compiler.c).

export MARIO_LANG=<lang>, <lang> is the compiler path under 'compilers' in this project. for exmaple: 'js'.
