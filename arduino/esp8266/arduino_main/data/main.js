pinMode(LED_BUILTIN, OUTPUT);

var i = 0;
while(i<100) {
	print("Hello, JS world!\n");
	i++;
	digitalWrite(LED_BUILTIN, LOW);
	delay(1000);
	digitalWrite(LED_BUILTIN, HIGH);
	delay(1000);
}

