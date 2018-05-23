#include "mario.h"
#include <stdio.h>

int main(int argc, char** argv) {
	const char* s = "var abc = f(123, 00);";
	while(true) {
		js_exec(s);
	}
	return 0;
}
