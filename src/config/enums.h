#ifndef ENUMS
#define ENUMS

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson

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