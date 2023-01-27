#ifndef ENUMS
#define ENUMS

#include <Arduino.h>
#include <ESP8266WiFi.h>


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
    
};

struct Mqtt {
  bool mqtt;
  char username[50];
  char password[50];
};




#endif