#ifndef ENUMS
#define ENUMS

#include <Arduino.h>

#if defined(ESP32) || defined(ESP32ETH)
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
  int charge;
  int dispo; 
  
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
/// @brief  température actuelle
float celsius=0.00; 
/// @brief  puissance actuelle en %
int puissance; 
/// @brief  puissance actuelle en Watt
int puissancewatt=0; 
/// @brief  puissance max locale en Watt
int puissancemax=0; 
/// @brief  puissance dispo en watt
int puissance_dispo=0;

int change=0; 
/// @brief état du ventilateur
bool cooler=0;
/// @brief  puissance cumulée en Watt (remonté par l'enfant toute les 10 secondes)
int puissance_cumul=0;
/// @brief etat de la surchauffe
byte security=0;
};

struct epoc {
  public:int heure;
  public:int minute;
  public:int seconde;
  public:int jour;
  public:int mois;
  public:int annee;
};




#endif