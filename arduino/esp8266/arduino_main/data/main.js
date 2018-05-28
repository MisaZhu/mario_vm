//Debug.dump(this);

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
if(ssl.connect("api.github.com", 443)) {
	print("SSL connected.\n");
}
else {
	print("SSL connect error!\n");
}

ssl.stop();
