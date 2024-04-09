#ifndef MINUTEUR_FUNCTIONS
#define MINUTEUR_FUNCTIONS

#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

#if defined(ESP32) || defined(ESP32ETH)
  #include <FS.h>
  #include "SPIFFS.h"
  #define LittleFS SPIFFS // Fonctionne, mais est-ce correct? 
#else
  #include <LittleFS.h>
#endif

//// NTP 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_OFFSET_SECONDS, NTP_UPDATE_INTERVAL_MS);

#include "config/enums.h"
#include "function/unified_dimmer.h"

extern System sysvar;
extern gestion_puissance unified_dimmer; 

void offset_heure_ete();
void timeclientEpoch_to_date(time_t epoch) ;

epoc actual_time; 

/// @brief ///////init du NTP 
void ntpinit() {
      // Configurer le serveur NTP et le fuseau horaire
  timeClient.begin();
  timeClient.update();
  //Serial.println(timeClient.getFormattedTime());
  offset_heure_ete();
  Serial.println(timeClient.getFormattedTime());
  
}

void timeclientEpoch_to_date(time_t epoch)  { // convert epoch to date  
  actual_time.mois = month(epoch);
  actual_time.jour = day(epoch);
  actual_time.heure = hour(epoch);
  DEBUG_PRINTLN(actual_time.mois);
  DEBUG_PRINTLN(actual_time.jour);
  DEBUG_PRINTLN(actual_time.heure);
  }


void offset_heure_ete() {
  timeclientEpoch_to_date(timeClient.getEpochTime());

              //detection été /hiver
            if (actual_time.mois > 10 || actual_time.mois < 3 
            || (actual_time.mois == 10 && (actual_time.jour) > 22 && (actual_time.weekday == 7)) 
            || (actual_time.mois == 3 && (actual_time.jour)<24 && actual_time.weekday == 7) ){
                //C'est l'hiver
                timeClient.setTimeOffset(NTP_OFFSET_SECONDS*1); 
                }
                else{
                //C'est l'été
                timeClient.setTimeOffset(NTP_OFFSET_SECONDS*2); 
            }
}

//////// structure pour les programmateurs. 
struct Programme {
  public:char heure_demarrage[6];
  public:char heure_arret[6];
  public:int temperature=50;
  public:bool run; 
  public:int heure;
  public:int minute;
  public:String name;
  public:int seuil_start;
  public:int seuil_stop;
  public:int seuil_temperature;
  private:bool security = false;


  /// @brief sauvegarde
  /// @param programme_conf 
  public:void saveProgramme() {
        const char * c_file = name.c_str();
        DynamicJsonDocument doc(256);

      ////vérification cohérence des données
      if (check_data(heure_demarrage)) {strcpy(heure_demarrage, "00:00"); }
      if (check_data(heure_arret)) {strcpy(heure_arret, "00:00"); }
      
        // Set the values in the document
        doc["heure_demarrage"] = heure_demarrage;
        doc["heure_arret"] = heure_arret;
        doc["temperature"] = temperature;
        doc["seuil_start"] = seuil_start;
        doc["seuil_stop"] = seuil_stop;
        doc["seuil_temperature"] = seuil_temperature;
                
          // Open file for writing
        File configFile = LittleFS.open(c_file, "w");
        if (!configFile) {
          Serial.println(F("Failed to open config file for writing"));
          return;
        }

        // Serialize JSON to file
        if (serializeJson(doc, configFile) == 0) {
          Serial.println(F("Failed to write to file"));
        }
        
        configFile.close();
  }

  /// @brief chargement
  /// @param programme_conf 
  

  public:bool loadProgramme() {
        const char * c_file = name.c_str();
        File configFile = LittleFS.open(c_file, "r");

        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/v6/assistant to compute the capacity.
        DynamicJsonDocument doc(256);

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, configFile);
        if (error) {
          Serial.println(F("Failed to read minuterie config "));
          return false;
        }
      
        strlcpy(heure_demarrage,                  // <- destination
                doc["heure_demarrage"] | "00:00", // <- source
                sizeof(heure_demarrage));         // <- destination's capacity
        
        strlcpy(heure_arret,                  // <- destination
                doc["heure_arret"] | "00:00", // <- source
                sizeof(heure_arret));         // <- destination's capacity
        temperature = doc["temperature"] | 50 ; /// defaut à 50 °
        seuil_start = doc["seuil_start"] | 0 ; /// defaut à 0 %°
        seuil_stop = doc["seuil_stop"] | 0 ; /// defaut à sans arret %
        seuil_temperature = doc["seuil_temperature"] | 0 ; /// defaut à 0 °

        configFile.close();
      return true;    
  }

 void commande_run(){
        run=true; 
        timeClient.update();

  }

bool start_progr() {
  /// test de la sécurité avant relance
  if (security && ( sysvar.celsius > float(temperature*0.95) ) )  { return false; }
  security = false;

  int heures, minutes;
  sscanf(heure_demarrage, "%d:%d", &heures, &minutes);

  int heures_fin, minutes_fin;
  sscanf(heure_arret, "%d:%d", &heures_fin, &minutes_fin);

  // si heure_demarrage == heure_arret alors on retourne false ( correction du bug si pas de programmation)
  if (strcmp(heure_demarrage, heure_arret) == 0) {
        return false;
  }
      
  if(timeClient.isTimeSet()) {
    if (heures == timeClient.getHours() && minutes == timeClient.getMinutes() && sysvar.celsius < temperature ) { // correction bug #19  
        commande_run();
        return true; 
    }
  }

  // remise en route en cas de reboot et si l'heure est dépassée  
  // recherche si l'heure est passée 
  bool heure_passee = false;
  if (timeClient.getHours() > heures || (timeClient.getHours() == heures && timeClient.getMinutes() > minutes )) {
                       heure_passee = true; 
  }
  // recherche si l'heure d'arret est est passée
  bool heure_arret_passee = false;
  if (timeClient.getHours() > heures_fin || (timeClient.getHours() == heures_fin && timeClient.getMinutes() >= minutes_fin )) {
                       heure_arret_passee = true; 
  }


  if (heure_passee && !heure_arret_passee && sysvar.celsius < temperature ) {
           commande_run();
            return true; 
  }


return false; 
}

bool stop_progr() {
  int heures, minutes;
  /// sécurité temp
  if ( sysvar.celsius >= temperature ) { 
    run=false; 
    security = true;
     // protection flicking
    sscanf(heure_demarrage, "%d:%d", &heures, &minutes);  

    if (heures == timeClient.getHours() && minutes == timeClient.getMinutes()) {
      delay(15000);
    }
  return true; 
  }

  sscanf(heure_arret, "%d:%d", &heures, &minutes);



  if(timeClient.isTimeSet()) {
    if (heures == timeClient.getHours() && minutes == timeClient.getMinutes()) {
        run=false; 
        timeClient.update();
        offset_heure_ete();     
        return true; 
    }
  }
  return false; 
}
    /// démarrage si le seuil est atteint 
  bool start_seuil() {
    if ( unified_dimmer.get_power() >= seuil_start && sysvar.celsius < seuil_temperature && seuil_start != seuil_stop) { 
      return true;
    }
  return false; 
  }

  /// arrêt si le seuil est atteint
  bool stop_seuil() {
    if ( unified_dimmer.get_power() >= seuil_stop && seuil_start != seuil_stop && sysvar.celsius > seuil_temperature) { 
      return true;
    }
  return false;
  }

  /// arret si seuil temp est atteint
  bool stop_seuil_temp() {
    if ( sysvar.celsius >= seuil_temperature && seuil_temperature != 0) { 
      return true;
    }
  return false;
  }

 /// vérification de la conformité de la donnée heure_demarrage[6]; 
 bool check_data(char data[6]){
  int heures, minutes;
  int result = sscanf(data, "%d:%d", &heures, &minutes);
  if (result != 2) {
    Serial.println("Erreur de lecture de l'heure");
    return true;
  }
  if (heures >= 0 && heures <= 23 && minutes >= 0 && minutes <= 59) {
      return false;
    }
  Serial.println("Erreur de lecture de l'heure");
  return true;
 }

};

#endif