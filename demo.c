#include "mario.h"
#include <stdio.h>

void dump(const char* s) {
	printf("%s", s);
}

int main(int argc, char** argv) {
	const char* s = "var abc = f(123, 0);";
	compile_t compile;

	compile_load(&compile, s);
	compile_run(&compile);

	compile_dump(&compile, dump);
	compile_close(&compile);
	return 0;
}
