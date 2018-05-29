//Debug.dump(this);

host = "api.github.com";
fprint = "35 85 74 EF 67 35 A7 CE 40 69 50 F3 C0 F6 80 CF 80 3B 2E 19";
ssid = "Misa.Test";
passwd = "test1234";

pinMode(LED_BUILTIN, OUTPUT);

function testWiFi() {
	print("Connecting WiFi");

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, passwd);

	while(WiFi.status() != WL_CONNECTED)  {
		digitalWrite(LED_BUILTIN, LOW);
		delay(200);
		digitalWrite(LED_BUILTIN, HIGH);
		delay(200);
		print(".");
	}
	print("\nIP:" + WiFi.localIP() + "\n");
}

function testSSL() {
	print("SSL connecting " + host + "..."); 

	ssl = new SSLClient();
	if(ssl.connect(host, 443)) {
		print("ok.\n");
	}
	else {
		print("error!\n");
		return;
	}

	print("SSL verifing...");
	if(ssl.verify(host, fprint)) {
		print("ok.\n");
	}
	else {
		print("error!\n");
		return;
	}

	url = "/repos/esp8266/Arduino/commits/master/status";
	s = "GET " + url + " HTTP/1.1\r\n" +
		"Host: " + host + "\r\n" +
		"User-Agent: BuildFailureDetectorESP8266\r\n" +
		"Connection: close\r\n\r\n";

	print("req: \n" + s);
	print("wrote: " + ssl.write(s, s.length) + "\n");

	while(ssl.connected())  {
		ready = ssl.available();
		if(ready == 0)
			continue;

		print("available: " + ready + "\n");

		while(true) {
			bytes = new Bytes(100);
			i = ssl.read(bytes);
			print("read: " + i + "\n");

			if(i <= 0) {
				break;
			}
			else {
				print(bytes.toString() + "\n");
				ready -= i; 
				if(ready <= 0)
					break;
			}
		}
	}

	ssl.stop();
	print("SSL closed.\n");
}

testWiFi();
testSSL();
