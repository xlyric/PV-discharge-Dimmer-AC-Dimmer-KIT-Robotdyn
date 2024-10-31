#ifndef TASK_PING
#define TASK_PING

#include "Arduino.h"
#include "config/enums.h"

#if defined(ESP32) || defined(ESP32ETH)
// Web services
  #include "WiFi.h"
  #include "HTTPClient.h"
#else
// Web services
  #include <ESP8266WiFi.h>
  //#include <ESPAsyncTCP.h>
  #include <ESP8266HTTPClient.h>
#endif

extern System sysvar;
extern WiFiClient domotic_client;


void ping() {
  /// test du ping du site internet
  HTTPClient http;
  http.begin(domotic_client, String(sysvar.pingurl),80, "/ping");
  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.printf("[HTTP PING] GET... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      sysvar.ping = true;
      sysvar.pingfail = 0;
    }
  } else {
    Serial.printf("[HTTP PING] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    sysvar.pingfail ++;
    if (sysvar.pingfail > 3 && sysvar.ping == true) {
      Serial.print("[HTTP PING] connexion lost reboot ");
      ESP.restart();
    }
  }
  http.end();
}

#endif

