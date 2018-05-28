#ifndef NATIVE_H
#define NATIVE_H

#include "mario.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

var_t* native_print(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "str");
	const char* s = n == NULL ? "" : var_get_str(n->var);
	Serial.print(s);
	return NULL;
}

var_t*  native_pinMode(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "pin");
	int pin = n == NULL ? 0 : var_get_int(n->var);

	n = var_find(env, "type");
	int type = n == NULL ? 0 : var_get_int(n->var);

	pinMode(pin, type);
	return NULL;
}

var_t*  native_digitalWrite(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "pin");
	int pin = n == NULL ? 0 : var_get_int(n->var);

	n = var_find(env, "type");
	int type = n == NULL ? 0 : var_get_int(n->var);

	digitalWrite(pin, type);
	return NULL;
}

var_t* native_delay(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "msec");
	int msec = n == NULL ? 0 : var_get_int(n->var);

	delay(msec);
	return NULL;
}

/*=====WiFi native functions=========*/
var_t* native_WiFiMode(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "md");
	int md = n == NULL ? 0 : var_get_int(n->var);

	WiFi.mode((WiFiMode_t)md);
	return NULL;
}

var_t* native_WiFiStatus(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)env; (void)data;

	return var_new_int(WiFi.status());
}

var_t* native_WiFiLocalIP(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)env; (void)data;
	str_t* ips = str_new("");

	IPAddress ip = WiFi.localIP();
	str_append(ips, str_from_int(ip[0])); str_add(ips, '.');
	str_append(ips, str_from_int(ip[1])); str_add(ips, '.');
	str_append(ips, str_from_int(ip[2])); str_add(ips, '.');
	str_append(ips, str_from_int(ip[3]));

	var_t* ret = var_new_str(ips->cstr);
	str_free(ips);
	return ret;
}

var_t* native_WiFiBegin(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* n = var_find(env, "ssid");
	const char* ssid = n == NULL ? "" : var_get_str(n->var);
	n = var_find(env, "passwd");
	const char* passwd = n == NULL ? "" : var_get_str(n->var);

	WiFi.begin(ssid, passwd);
	return NULL;
}


/*=====SSLClient native functions=========*/
void httpsClientFree(void*p) {
	WiFiClientSecure* clt = (WiFiClientSecure*)p;
	delete clt;
}

var_t* native_SSLClientConstructor(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	node_t* thisN = var_find(env, THIS);
	if(thisN == NULL)
		return NULL;

	WiFiClientSecure* clt = new WiFiClientSecure();
	var_t* cltVar = var_new_object(clt, httpsClientFree);
	var_add(thisN->var, "httpsClient", cltVar);
	return thisN->var;
}

WiFiClientSecure* getSSLClient(var_t* env) {
	node_t* thisN = var_find(env, THIS);
	if(thisN == NULL)
		return NULL;
	node_t* cltNode = var_find(thisN->var, "httpsClient");
	if(cltNode == NULL)
		return NULL;
	return (WiFiClientSecure*)cltNode->var->value;	
}

var_t* native_SSLClientConnect(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	WiFiClientSecure* clt = getSSLClient(env);
	if(clt == NULL)
		return NULL;

	node_t* n = var_find(env, "host");
	const char* host = n == NULL ? "" : var_get_str(n->var);
	n = var_find(env, "port");
	int port = n == NULL ? 0 : var_get_int(n->var);

	bool res = clt->connect(host, port);
	return var_new_int(res);
}

var_t* native_SSLClientVerify(vm_t* vm, var_t* env, void* data) {
  (void)vm; (void)data;
  WiFiClientSecure* clt = getSSLClient(env);
  if(clt == NULL)
    return NULL;

  node_t* n = var_find(env, "host");
  const char* host = n == NULL ? "" : var_get_str(n->var);
  n = var_find(env, "fprint");
  const char* fprinter = n == NULL ? "" : var_get_str(n->var);

  bool res = clt->verify(fprinter, host);
  return var_new_int(res);
}

var_t* native_SSLClientConnected(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	WiFiClientSecure* clt = getSSLClient(env);
	if(clt == NULL)
		return NULL;

	bool res = clt->connected();
	return var_new_int(res);
}

var_t* native_SSLClientWrite(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	WiFiClientSecure* clt = getSSLClient(env);
	if(clt == NULL)
		return NULL;

	node_t* n = var_find(env, "bytes");
	if(n == NULL || n->var == NULL || n->var->size == 0)
		return NULL;
	var_t* bytes = n->var;

	n = var_find(env, "size");
	int size = n == NULL ? 0 : var_get_int(n->var);
	if(size > (int)bytes->size)
		size = bytes->size;

	int res = clt->write((const uint8_t*)bytes->value, size);
	return var_new_int(res);
}

var_t* native_SSLClientRead(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	WiFiClientSecure* clt = getSSLClient(env);
	if(clt == NULL)
		return NULL;

	node_t* n = var_find(env, "bytes");
	if(n == NULL || n->var == NULL || n->var->size == 0)
		return NULL;

  var_t* bytes = n->var;
	int res = clt->read((uint8_t*)bytes->value, n->var->size);
	return var_new_int(res);
}

var_t* native_SSLClientAvailable(vm_t* vm, var_t* env, void* data) {
  (void)vm; (void)data;
  WiFiClientSecure* clt = getSSLClient(env);
  if(clt == NULL)
    return NULL;

  int i = clt->available();
  return var_new_int(i);
}

var_t* native_SSLClientStop(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	WiFiClientSecure* clt = getSSLClient(env);
	if(clt == NULL)
		return NULL;

	clt->stop();
	return NULL;
}

#define CLS_WIFI "WiFi"
#define CLS_SSL_CLIENT "SSLClient"

void reg_native(vm_t* vm) {
	vm_reg_var(vm, "", "LED_BUILTIN", var_new_int(LED_BUILTIN));
	vm_reg_var(vm, "", "OUTPUT", var_new_int(OUTPUT));
	vm_reg_var(vm, "", "LOW", var_new_int(LOW));
	vm_reg_var(vm, "", "HIGH", var_new_int(HIGH));

	vm_reg_native(vm, "", "print(str)", native_print, NULL);
	vm_reg_native(vm, "", "delay(msec)", native_delay, NULL);
	vm_reg_native(vm, "", "pinMode(pin, type)", native_pinMode, NULL);
	vm_reg_native(vm, "", "digitalWrite(pin, type)", native_digitalWrite, NULL);

	vm_reg_var(vm, "", "WIFI_STA", var_new_int(WIFI_STA));
	vm_reg_var(vm, "", "WL_CONNECTED", var_new_int(WL_CONNECTED));
	vm_reg_native(vm, CLS_WIFI, "mode(md)", native_WiFiMode, NULL);
	vm_reg_native(vm, CLS_WIFI, "begin(ssid, passwd)", native_WiFiBegin, NULL);
	vm_reg_native(vm, CLS_WIFI, "status()", native_WiFiStatus, NULL);
	vm_reg_native(vm, CLS_WIFI, "localIP()", native_WiFiLocalIP, NULL);

	vm_reg_native(vm, CLS_SSL_CLIENT, "constructor()", native_SSLClientConstructor, NULL);
	vm_reg_native(vm, CLS_SSL_CLIENT, "stop()", native_SSLClientStop, NULL);
	vm_reg_native(vm, CLS_SSL_CLIENT, "connect(host, port)", native_SSLClientConnect, NULL);
  vm_reg_native(vm, CLS_SSL_CLIENT, "verify(host, fprint)", native_SSLClientVerify, NULL);
	vm_reg_native(vm, CLS_SSL_CLIENT, "connected()", native_SSLClientConnected, NULL);
  vm_reg_native(vm, CLS_SSL_CLIENT, "available()", native_SSLClientAvailable, NULL);
	vm_reg_native(vm, CLS_SSL_CLIENT, "write(bytes, size)", native_SSLClientWrite, NULL);
	vm_reg_native(vm, CLS_SSL_CLIENT, "read(bytes)", native_SSLClientRead, NULL);
}

#endif

