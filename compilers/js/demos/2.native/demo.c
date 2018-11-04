#include "compiler.h"

const char* js = " \
	out(\"Hello, global native.\n\"); \
	ClassHello.out(ClassHello.HELLO); \
	";

var_t* native_out(vm_t* vm, var_t* env, void* data) {
	const char* s = get_str(env, "str");
	_out_func(s);
	return NULL;
}

int main(int argc, char** argv) {
	vm_t* vm = vm_new();
	vm_init(vm, compiler, NULL, NULL);

	/** Register a native function(mapped to js) in class 'ClassHello'.
		Class name: ClassHello (doesn't exist, so will be created automaticly).
		function name: out
		argument: str
	*/
	vm_reg_native(vm, "ClassHello", "out(str)", native_out, NULL);

	/** Register a native global function(mapped to js).
		Class name: none (global function).
		function name: out
		argument: str
	*/
	vm_reg_native(vm, "", "out(str)", native_out, NULL);

	/** Register a const variable 'HELLO' in class 'ClassHello'. */
	vm_reg_var(vm, "ClassHello", "HELLO", var_new_str("Hello, class native.\n"), true);

	vm_load_run(vm, js);

	vm_close(vm);
	return 0;
}
