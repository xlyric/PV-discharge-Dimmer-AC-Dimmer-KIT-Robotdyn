#ifndef DIMMER_FUNCTIONS
#define DIMMER_FUNCTIONS

///regroupement des fonctions de dimmer

#include <Arduino.h>
#include "mqtt.h"
#ifdef ROBOTDYN
  #include <RBDdimmer.h>
#endif

#if defined(ESP32) || defined(ESP32ETH)
// Web services
  #include "WiFi.h"
  #include <AsyncTCP.h>
  #include "HTTPClient.h"
#else
// Web services
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESP8266HTTPClient.h> 
#endif

extern struct System sysvar;
extern struct Config config;  
extern dimmerLamp dimmer;
#ifdef outputPin2
extern dimmerLamp dimmer2;
#endif
#ifdef outputPin3
extern dimmerLamp dimmer3;
#endif


#endif