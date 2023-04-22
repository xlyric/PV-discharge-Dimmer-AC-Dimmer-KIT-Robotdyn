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
  int dimmer_on_off;

};

struct Mqtt {
  public:bool mqtt;
  public:bool HA;
  public:bool jeedom;
  public:bool domoticz;
  public:char username[50];
  public:char password[50];
};

struct Wifi_struct {
  public:char static_ip[16];
  public:char static_sn[16];
  public:char static_gw[16];
};

///variables globales 
struct System {

float celsius=0.00; 
int puissance; 
int change=0; 
//String loginit;
//String logs="197}11}1";

};

struct Programme {
public:char heure_demarrage[6];
public:char heure_arret[6];
public:int temperature;
public:bool run; 
public:int heure;
public:int minute;



};



#endif