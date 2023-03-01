#ifndef ENUMS
#define ENUMS

#include <Arduino.h>
#ifdef ESP32
  #include "WiFi.h"
#else
  #include <ESP8266WiFi.h>
#endif

extern String logs;

struct Config {
  char hostname[16];
  int port;
  char Publish[100];
  int IDXTemp;
  int maxtemp;
  int IDXAlarme;
  int IDX;
  int maxpow;
  char child[16];
  char mode[10];
  int minpow;
  int startingpow;
  char SubscribePV[100];
  char SubscribeTEMP[100];
  bool restart;
  int dimmer_on_off = 1 ;
  bool HA;
  bool JEEDOM;
  bool DOMOTICZ;

};

struct Mqtt {
  public:bool mqtt;
  public:char username[50];
  public:char password[50];
};

///variables globales 
struct System {

float celsius=0.00; 
int puissance; 
int change=0; 
//String loginit;
//String logs="197}11}1";

};


#endif