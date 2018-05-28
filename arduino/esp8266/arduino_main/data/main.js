//Debug.dump(this);

host = "api.github.com";

pinMode(LED_BUILTIN, OUTPUT);

print("Connecting WiFi");

WiFi.mode(WIFI_STA);
WiFi.begin("Misa.Test", "test1234");

while(WiFi.status() != WL_CONNECTED)  {
	digitalWrite(LED_BUILTIN, LOW);
	delay(200);
	digitalWrite(LED_BUILTIN, HIGH);
	delay(200);
	print(".");
}

print("\nconnected with IP:" + WiFi.localIP() + "\n");

print("SSL connecting api.github.com: 443.\n"); 

ssl = new SSLClient();
if(ssl.connect(host, 443)) {
	print("SSL connected.\n");
}
else {
	print("SSL connect error!\n");
}

url = "/repos/esp8266/Arduino/commits/master/status";
s = "GET " + url + " HTTP/1.1\r\n" +
				"Host: " + host + "\r\n" +
				"User-Agent: BuildFailureDetectorESP8266\r\n" +
			 "Connection: close\r\n\r\n";

print("req: \n" + s);
bytes = new Bytes();
bytes.fromString(s);

print("wrote: " + ssl.write(bytes, bytes.size()) + "\n");

bytes = new Bytes(200);

while (ssl.connected())  {
	i = ssl.read(bytes);
	print("read: " + i + "\n");
	if(i <= 0)
		break;
	else {
		print(bytes.toString());
	}
}

ssl.stop();
