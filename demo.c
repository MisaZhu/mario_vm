#include "mario.h"
#include <stdio.h>

int main(int argc, char** argv) {
	const char* s = "var abc = f(123, 00);";
	js_load(s);
	js_run();
	js_close();
	return 0;
}
