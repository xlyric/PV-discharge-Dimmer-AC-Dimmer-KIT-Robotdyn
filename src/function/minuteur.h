#ifndef MINUTEUR_FUNCTIONS
#define MINUTEUR_FUNCTIONS

#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson

#include <WiFiUdp.h>
#include "config/enums.h"

#if defined(ESP32) || defined(ESP32ETH)
  #include <FS.h>
  #include "SPIFFS.h"
  #define LittleFS SPIFFS // Fonctionne, mais est-ce correct?
#else
  #include <LittleFS.h> // NOSONAR
#endif

#include "config/enums.h"
#include "function/unified_dimmer.h"

extern System sysvar;
extern Config config;
extern gestion_puissance unified_dimmer;

struct tm timeinfo;
epoc actual_time;

/// @brief /////// init du NTP
void ntpinit() {
  // Configurer le serveur NTP et le fuseau horaire
  // Voir Time-Zone: https://sites.google.com/a/usapiens.com/opnode/time-zones
  configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", NTP_SERVER);
  getLocalTime( &timeinfo );
  Serial.println(asctime(&timeinfo));

}

//////// structure pour les programmateurs.
struct Programme {
public:
  char heure_demarrage[6];  // NOSONAR
  char heure_arret[6];  // NOSONAR
  int temperature=50;
  bool run;
  int heure;
  int minute;

  int seuil_start;
  int seuil_stop;
  int seuil_temperature;
  int puissance=100;

private:
  bool security = false;
  String name;

/// setter pour le nom du programme
public: void set_name(String name) {
    this->name = name;
  }


/// @brief sauvegarde
/// @param programme_conf
public: void saveProgramme() {
    const char * c_file = name.c_str();    // NOSONAR
    JsonDocument doc;

    //// vérification cohérence des données
    if (check_data(heure_demarrage)) {strcpy(heure_demarrage, "00:00"); }
    if (check_data(heure_arret)) {strcpy(heure_arret, "00:00"); }

    // Set the values in the document
    doc["heure_demarrage"] = heure_demarrage;
    doc["heure_arret"] = heure_arret;
    doc["temperature"] = temperature;
    doc["seuil_start"] = seuil_start;
    doc["seuil_stop"] = seuil_stop;
    doc["seuil_temperature"] = seuil_temperature;
    doc["puissance"] = puissance;

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


public: bool loadProgramme() {
    const char * c_file = name.c_str();    // NOSONAR
    File configFile = LittleFS.open(c_file, "r");

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
      Serial.println(F("Failed to read minuterie config "));
      return false;
    }

    strlcpy(heure_demarrage,                           // <- destination
            doc["heure_demarrage"] | "00:00",          // <- source
            sizeof(heure_demarrage));                  // <- destination's capacity

    strlcpy(heure_arret,                               // <- destination
            doc["heure_arret"] | "00:00",              // <- source
            sizeof(heure_arret));                      // <- destination's capacity
    temperature = doc["temperature"] | 50;             // defaut à 50 °
    seuil_start = doc["seuil_start"] | 0;              // defaut à 0 %°
    seuil_stop = doc["seuil_stop"] | 0;                // defaut à sans arret %
    seuil_temperature = doc["seuil_temperature"] | 0;  // defaut à 0 °
    puissance = doc["puissance"] | 100;                // defaut à 100 %

    configFile.close();
    return true;
  }

  void commande_run(){
    run=true;

  }

  bool start_progr() {
    /// test de la sécurité avant relance
    if (security && ( sysvar.celsius[sysvar.dallas_maitre]> float(temperature*0.95) ) )  { return false; }
    security = false;

    int heures;
    int minutes;

    sscanf(heure_demarrage, "%d:%d", &heures, &minutes);

    int heures_fin;
    int minutes_fin;

    sscanf(heure_arret, "%d:%d", &heures_fin, &minutes_fin);

    // si heure_demarrage == heure_arret alors on retourne false ( correction du bug si pas de programmation)
    if (strcmp(heure_demarrage, heure_arret) == 0) {
      return false;
    }

    if(getLocalTime(&timeinfo)) {
      if (heures == timeinfo.tm_hour && minutes == timeinfo.tm_min &&
          sysvar.celsius[sysvar.dallas_maitre]< temperature ) {  // correction bug #19
        commande_run();
        return true;
      }
    }

    // remise en route en cas de reboot et si l'heure est dépassée
    // recherche si l'heure est passée
    bool heure_passee = false;
    if (timeinfo.tm_hour > heures || (timeinfo.tm_hour == heures && timeinfo.tm_min > minutes )) {
      heure_passee = true;
    }
    // recherche si l'heure d'arret est est passée
    bool heure_arret_passee = false;
    if (timeinfo.tm_hour > heures_fin || (timeinfo.tm_hour == heures_fin && timeinfo.tm_min >= minutes_fin )) {
      heure_arret_passee = true;
    }

    if (heure_passee && !heure_arret_passee && sysvar.celsius[sysvar.dallas_maitre]< temperature ) {
      commande_run();
      return true;
    }
    return false;
  }

  bool stop_progr() {
    int heures;
    int minutes;
    /// sécurité temp
    if ( sysvar.celsius[sysvar.dallas_maitre]>= temperature ) {
      run=false;
      security = true;
      // protection flicking
      sscanf(heure_demarrage, "%d:%d", &heures, &minutes);

      if (heures == timeinfo.tm_hour && minutes == timeinfo.tm_min) {
        delay(15000);
      }
      return true;
    }

    sscanf(heure_arret, "%d:%d", &heures, &minutes);



    if(getLocalTime(&timeinfo)) {
      if (heures == timeinfo.tm_hour && minutes == timeinfo.tm_min) {
        run=false;
        return true;
      }
    }
    return false;
  }
  /// démarrage si le seuil est atteint
  bool start_seuil() {
    if ( unified_dimmer.get_power() >= seuil_start && sysvar.celsius[sysvar.dallas_maitre]< seuil_temperature &&
         seuil_start != seuil_stop) {
      return true;
    }
    return false;
  }

  /// arrêt si le seuil est atteint
  bool stop_seuil() {
    if ( unified_dimmer.get_power() >= seuil_stop && seuil_start != seuil_stop &&
         sysvar.celsius[sysvar.dallas_maitre]> seuil_temperature) {
      return true;
    }
    return false;
  }

  /// arret si seuil temp est atteint
  bool stop_seuil_temp() {
    if ( sysvar.celsius[sysvar.dallas_maitre]>= seuil_temperature && seuil_temperature != 0) {
      return true;
    }
    return false;
  }

  /// vérification de la conformité de la donnée heure_demarrage[6];
  bool check_data(char data[6]){
    int heures;
    int minutes;
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