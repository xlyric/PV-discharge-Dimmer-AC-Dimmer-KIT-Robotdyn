#ifndef ENUMS
#define ENUMS

#include <Arduino.h>

#if defined(ESP32) || defined(ESP32ETH)
  #include "WiFi.h"
#else
  #include <ESP8266WiFi.h>
#endif

//extern String logs;

/// @brief  partie délicate car pas mal d'action sur la variable log_init et donc protection de la variable ( pour éviter les pb mémoire )
struct Logs {
private:
      char log_init[LOG_MAX_STRING_LENGTH];

public:
  ///setter log_init
    void Set_log_init(String setter) { 
        // Vérifier si la longueur de la chaîne ajoutée ne dépasse pas LOG_MAX_STRING_LENGTH
        if ( strlen(setter.c_str()) + strlen(log_init) < LOG_MAX_STRING_LENGTH)  { 
          strcat(log_init,setter.c_str());  
        } else {  
          // Si la taille est trop grande, réinitialiser le log_init
          log_init[0] = '\0';
          strcat(log_init,"197}11}1");
        }     
      }

    ///getter log_init
    String Get_log_init() {return log_init; }

    //clean log_init
    void clean_log_init() {
        // Vérifier si la longueur de log_init dépasse 80% de LOG_MAX_STRING_LENGTH
        if (strlen(log_init) > (LOG_MAX_STRING_LENGTH - (LOG_MAX_STRING_LENGTH/5)) ) {
        log_init[0] = '\0';
        strcat(log_init,"197}11}1");
        }
    }

  //reset log_init
    void reset_log_init() {
      log_init[0] = '\0';
      strcat(log_init,"197}11}1");
    }

  
};


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
/// @brief  // Somme des 3 charges déclarées dans la page web
  int charge;
/// @brief  // Puissance de la charge 1 déclarée dans la page web
  int charge1; 
/// @brief  // Puissance de la charge 2 déclarée dans la page web
  int charge2; 
/// @brief  // Puissance de la charge 3 déclarée dans la page web
  int charge3; 
  int dispo; 
  char say_my_name[32];
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
float celsius=20.00; 
/// @brief  puissance actuelle en %
float puissance; 
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
  public:int weekday;
};




#endif