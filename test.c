#include "mario.h"
#include <stdio.h>

int main(int argc, char** argv) {
	const char* s = "var a = 1;";
	js_exec(s);
	return 0;
}
