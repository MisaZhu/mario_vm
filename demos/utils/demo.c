#include "mario_utils.h"
#include <stdio.h>

int main(int argc, char** argv) {
	utf8_t* u = utf8_new("我们的abcd");
	utf8_append(u, "他们的xfg");
	str_t s;
	utf8_to_str(u, &s);
	printf("[%d: %s]\n", utf8_len(u), s.cstr);
	utf8_free(u);

	return 0;
}
