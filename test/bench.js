a = 0;

while(a < 3000000) {
	a = a + 1;
	if((a % 100000) == 0)
		println("" + a);
}
