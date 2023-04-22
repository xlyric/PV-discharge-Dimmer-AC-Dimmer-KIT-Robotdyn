#ifndef MINUTEUR_FUNCTIONS
#define MINUTEUR_FUNCTIONS

#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

#ifdef ESP32
  #include <FS.h>
  #include "SPIFFS.h"
  #define LittleFS SPIFFS // Fonctionne, mais est-ce correct? 
#else
  #include <LittleFS.h>
#endif

//// NTP 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

#include "config/enums.h"
extern Programme programme; 
extern System sysvar;

void ntpinit() {
      // Configurer le serveur NTP et le fuseau horaire
  timeClient.begin();
  timeClient.setTimeOffset(3600); // Fuseau horaire (en secondes, ici GMT+1)
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  Serial.println(programme.heure_demarrage);
  Serial.println(programme.heure_arret); 

}




bool start_progr() {
  int heures, minutes;
  sscanf(programme.heure_demarrage, "%d:%d", &heures, &minutes);
      
  if(timeClient.isTimeSet()) {
    if (heures == timeClient.getHours() && minutes == timeClient.getMinutes()) {
        programme.run=true; 
        timeClient.update();
        return true; 
    }
  }
return false; 
}

bool stop_progr() {
  int heures, minutes;
  /// sécurité temp
  if ( sysvar.celsius >= programme.temperature ) { 
    programme.run=false; 
     // protection flicking
    sscanf(programme.heure_demarrage, "%d:%d", &heures, &minutes);  
    if (heures == timeClient.getHours() && minutes == timeClient.getMinutes()) {
      delay(15000);
    }
  return true; 
  }
  
  sscanf(programme.heure_arret, "%d:%d", &heures, &minutes);
  if(timeClient.isTimeSet()) {
    if (heures == timeClient.getHours() && minutes == timeClient.getMinutes()) {
        programme.run=false; 
        timeClient.update();
        return true; 
    }
  }
  return false; 
}

#endif