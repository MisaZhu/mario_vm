#include "mario.h"
#include <stdio.h>

int main(int argc, char** argv) {
	str_t str;
	str_init(&str);
	str_reset(&str);
	str_release(&str);
	return 0;
}
