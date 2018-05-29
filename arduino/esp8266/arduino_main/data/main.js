//Debug.dump(this);

host = "api.github.com";
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

function testHTTP() {
	http = new HTTPClient();

	print("http connecting...");
	if(!http.begin("http://www.mock-server.com/")) {
		print("error!\n");
		return;
	}
	print("ok.\n");

	code = http.GET();
	print("GET code:" + code + "\n");

	if(code > 0) {
		while(http.connected()) {
			ready = http.available();
			if(ready == 0)
				continue;

			while(true) {
				bytes = new Bytes(100);
				i = http.read(bytes);

				if(i <= 0) {
					break;
				}
				else {
					print(bytes.toString());
					ready -= i; 
					if(ready <= 0)
						break;
				}
			}
		}
	}

	/*
	if(code > 0) {
		s = http.getString();
		print(s);
	}
	*/

	http.end();
}

testWiFi();
testHTTP();
