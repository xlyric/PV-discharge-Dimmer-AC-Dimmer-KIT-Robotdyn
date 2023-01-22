#ifndef ENUMS
#define ENUMS

#include <Arduino.h>

struct Config {
  char hostname[15];
  int port;
  char Publish[100];
  int IDXTemp;
  int maxtemp;
  int IDXAlarme;
  int IDX;
  int maxpow;
  char child[15];
  char mode[10];
  int minpow;
  int startingpow;
  char SubscribePV[100];
  char SubscribeTEMP[100];

    
};

struct Mqtt {
  bool mqtt;
  char username[50];
  char password[50];
};






#endif