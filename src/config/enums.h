#ifndef ENUMS
#define ENUMS

#include <Arduino.h>

#if defined(ESP32) || defined(ESP32ETH)
  #include "WiFi.h"
#else
  #include <ESP8266WiFi.h>
#endif

#if defined(ESP32) || defined(ESP32ETH)
  #include <FS.h>
  #include "SPIFFS.h"
  #define LittleFS SPIFFS // Fonctionne, mais est-ce correct?
#else
  #include <LittleFS.h> // NOSONAR
#endif

constexpr const int MAX_DALLAS=8; // nombre de sonde Dallas


/// @brief  partie délicate car pas mal d'action sur la variable log_init et donc protection de la variable ( pour éviter les pb mémoire )
struct Logs {
private:

  char log_init[LOG_MAX_STRING_LENGTH];     // NOSONAR
  int MaxString = LOG_MAX_STRING_LENGTH * .9;
  String alerte_web = "";

public:
  /// setter alerte_web
  void Set_alerte_web(String setter) {
    alerte_web = setter;
  }
  /// getter alerte_web
  String Get_alerte_web() {
    return alerte_web;
  }


/// setter log_init
public: void Set_log_init(String setter) {
    // Vérifier si la longueur de la chaîne ajoutée ne dépasse pas LOG_MAX_STRING_LENGTH
    if ( strlen(setter.c_str()) + strlen(log_init) < static_cast<size_t>(MaxString) )  {
      strcat(log_init,setter.c_str());
    } else {
      // Si la taille est trop grande, réinitialiser le log_init
      reset_log_init();
    }
  }

  /// getter log_init
  String Get_log_init() {
    return log_init;
  }

  // clean log_init
  void clean_log_init() {
    // Vérifier si la longueur de log_init dépasse 90% de LOG_MAX_STRING_LENGTH
    if (strlen(log_init) > static_cast<size_t>(MaxString) ) {
      reset_log_init();
    }
  }

  // reset log_init
  void reset_log_init() {
    log_init[0] = '\0';
    strcat(log_init,"197}11}1");
  }

  void log(String message, ...) {
    va_list args;
    va_start(args, message);
    char buffer[150];
    if (sizeof(args) > 0) {
      vsnprintf(buffer, sizeof(buffer), message.c_str(), args);
      Set_log_init(get_current_time() + " - " + String(buffer) + "\r\n");
    }
    else {
      Set_log_init(get_current_time() + " - " + message + "\r\n");
    }
    va_end(args);
  }

  String get_current_time(bool day=false) {
    static char uptime_stamp[20];   // Vous devrez définir une taille suffisamment grande pour stocker votre temps
    time_t maintenant;
    time(&maintenant);
    if (day) {
      strftime(uptime_stamp, sizeof(uptime_stamp), "%d/%m/%Y %H:%M:%S", localtime(&maintenant));
    } else {
      strftime(uptime_stamp, sizeof(uptime_stamp), "%H:%M:%S", localtime(&maintenant));
    }
    return String(uptime_stamp);
  }

};


struct Config {

public:
  const char *filename_conf = "/config.json";
  char hostname[16] = "192.168.1.22";   // NOSONAR
  int port = 1883;
  char Publish[100] = "domoticz/in";   // NOSONAR
  int IDXTemp = 200;
  int IDXAlarme = 202;
  int IDX = 201;
  int maxtemp = 60;
  int maxpow = 50;
  char child[64] = "";
  char mode[10] = "off";
  int minpow = 5;
  int startingpow = 0;
  char SubscribePV[100] = "none";
  char SubscribeTEMP[100] ="none";    // NOSONAR
  bool restart;
  bool dimmer_on_off = true;
  /// @brief  // Somme des 3 charges déclarées dans la page web
  int charge = 1000;
  /// @brief  // Puissance de la charge 1 déclarée dans la page web
  int charge1 = 1000;
  /// @brief  // Puissance de la charge 2 déclarée dans la page web
  int charge2 = 0;
  /// @brief  // Puissance de la charge 3 déclarée dans la page web
  int charge3 = 0;
  int dispo = 0;
  bool HA = true;
  bool JEEDOM = true;
  bool DOMOTICZ = true;
  char PVROUTER[5] = "mqtt";
  char DALLAS[17]   = "none";
  char say_my_name[32] = "";    // NOSONAR
  int trigger = 0;

  void check_trigger() {
    if (trigger < 0) { trigger = 0; }
    if (trigger > 100) { trigger = 100; }
  }

// Loads the configuration from a file
  String loadConfiguration() {
    String message = Load_configuration_loaded;
    // Open file for reading
    File configFile = LittleFS.open(filename_conf, "r");

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
      Serial.println(F("Failed to read configuration file, using default configuration"));
      message = Load_configuration_failed_use_default;
    }
    // Copy values from the JsonDocument to the Config

    /// en cas de reboot étranges, il sera bon de passer sur un format de type doc["hostname"].as<String>().c_str() pour éviter les problèmes de mémoire
    /// --> Exception 9: LoadStoreAlignmentCause: Load or store to an unaligned address. cas survenu avec les configuration MQTT
    String hostnamevalue = doc["hostname"].as<String>();
    strlcpy(hostname, hostnamevalue.c_str(), sizeof(hostname));

    port = doc["port"] | 1883;

    String PublishValue = doc["Publish"].as<String>();
    strlcpy(Publish, PublishValue.c_str(), sizeof(Publish));

    IDXTemp = doc["IDXTemp"] | 200;
    maxtemp = doc["maxtemp"] | 60;
    IDXAlarme = doc["IDXAlarme"] | 202;
    IDX = doc["IDX"] | 201;
    startingpow = doc["startingpow"] | 0;
    minpow = doc["minpow"] | 5;
    maxpow = doc["maxpow"] | 50;
    charge = doc["charge"] | 1000;
    charge1 = doc["charge1"] | 1000;
    charge2 = doc["charge2"] | 0;
    charge3 = doc["charge3"] | 0;
    trigger = doc["trigger"] | 10;
    check_trigger();

    String Publishchild = doc["child"].as<String>();
    strlcpy(child, Publishchild.c_str(), sizeof(child));

    String Publishmode = doc["mode"].as<String>();
    strlcpy(mode, Publishmode.c_str(), sizeof(mode));

    String PublishSubscribePV = doc["SubscribePV"].as<String>();
    strlcpy(SubscribePV, PublishSubscribePV.c_str(), sizeof(SubscribePV));

    String PublishSubscribeTEMP = doc["SubscribeTEMP"].as<String>();
    strlcpy(SubscribeTEMP, PublishSubscribeTEMP.c_str(), sizeof(SubscribeTEMP));

    dimmer_on_off = doc["dimmer_on_off"] | 1;
    HA = doc["HA"] | true;
    JEEDOM = doc["JEEDOM"] | true;
    DOMOTICZ = doc["DOMOTICZ"] | true;

    String PublishPVROUTER = doc["PVROUTER"].as<String>();
    strlcpy(PVROUTER, PublishPVROUTER.c_str(), sizeof(PVROUTER));

    String PublishDALLAS = doc["DALLAS"].as<String>();
    strlcpy(DALLAS, PublishDALLAS.c_str(), sizeof(DALLAS));

    String Publishsay_my_name = doc["name"].as<String>();
    if (strcmp(Publishsay_my_name.c_str(), "") == 0 || strcmp(Publishsay_my_name.c_str(), "null" ) == 0 ) {
      strcpy(say_my_name, ("dimmer-"+WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17)).c_str());
    }
    else strlcpy(say_my_name, Publishsay_my_name.c_str(), sizeof(say_my_name));

    configFile.close();

    return message;
  }

//***********************************
//************* Gestion de la configuration - sauvegarde du fichier de configuration
//***********************************

  String saveConfiguration() {
    String message = Save_configuration_saved;
    check_trigger();
    // Open file for writing
    File configFile = LittleFS.open(filename_conf, "w");
    if (!configFile) {
      Serial.println(F("Failed to open config file for writing"));
      return Save_configuration_failed_open;
    }

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/assistant to compute the capacity.
    JsonDocument doc;

    // Set the values in the document
    doc["hostname"] = hostname;
    doc["port"] = port;
    doc["Publish"] = Publish;
    doc["IDXTemp"] = IDXTemp;
    doc["maxtemp"] = maxtemp;
    doc["IDXAlarme"] = IDXAlarme;
    doc["IDX"] = IDX;
    doc["startingpow"] = startingpow;
    doc["minpow"] = minpow;
    doc["maxpow"] = maxpow;
    doc["child"] = child;
    doc["mode"] = mode;
    doc["SubscribePV"] = SubscribePV;
    doc["SubscribeTEMP"] = SubscribeTEMP;
    doc["dimmer_on_off"] = dimmer_on_off;
    doc["charge"] = charge;
    doc["HA"] = HA;
    doc["JEEDOM"] = JEEDOM;
    doc["DOMOTICZ"] = DOMOTICZ;
    doc["PVROUTER"] = PVROUTER;
    doc["DALLAS"] = DALLAS;
    doc["name"] = say_my_name;
    doc["charge1"] = charge1;
    doc["charge2"] = charge2;
    doc["charge3"] = charge3;
    doc["trigger"] = trigger;

    // Serialize JSON to file
    if (serializeJson(doc, configFile) == 0) {
      Serial.println(F("Failed to write to file"));
      message = Save_configuration_failed_write;
    }

    // Publish on MQTT
    // char buffer[1024];// NOSONAR
    // serializeJson(doc, buffer);
    // client.publish(("Xlyric/sauvegarde/"+ node_id).c_str() ,0,true, buffer);

    // Close the file
    configFile.close();
    return message;
  }

  void calcul_charge() {
    charge = charge1 + charge2 + charge3;
  }

};

///// structure MQTT

struct Mqtt {
public:
  const char *filename_mqtt = "/mqtt.json";
  bool mqtt;
  char username[64] = "mosquitto";   // NOSONAR
  char password[64] = "mosquitto";   // NOSONAR

  String loadmqtt() {
    // Open file for reading
    File configFile = LittleFS.open(filename_mqtt, "r");

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
      Serial.println(F("Failed to read MQTT config"));
      return "Failed to read MQTT config";
    }

    // Copy values from the JsonDocument to the Config
    String usernameValue = doc["MQTT_USER"].as<String>();
    strlcpy(username, usernameValue.c_str(), sizeof(username));

    String passwordValue = doc["MQTT_PASSWORD"].as<String>();
    strlcpy(password, passwordValue.c_str(), sizeof(password));

    mqtt = doc["mqtt"] | true;

    configFile.close();

    return "Configuration MQTT chargée.";
  }

  String savemqtt() {
    // Open file for writing
    File configFile = LittleFS.open(filename_mqtt, "w");
    String message = Save_configuration_saved;
    if (!configFile) {
      Serial.println(F("Failed to open config file for writing in function Save configuration"));
      return Save_configuration_failed_open;
    }

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/assistant to compute the capacity.
    JsonDocument doc;

    // Set the values in the document
    doc["MQTT_USER"] = username;
    doc["MQTT_PASSWORD"] = password;
    doc["mqtt"] = mqtt;
    if (serializeJson(doc, configFile) == 0) {
      Serial.println(F("Failed to write to file in function Save configuration "));
      message = Save_configuration_failed_write;
    }

    // Close the file
    configFile.close();
    // config.restart = true; /// à voir si on rajoute pour reboot après config MQTT
    return message;
  }


};

struct Wifi_struct {
public:
  char static_ip[16];
  char static_sn[16];
  char static_gw[16];
};

/// variables globales
struct System {
  /// @brief  température actuelle
  float celsius[MAX_DALLAS] = {0.00};
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
  bool cooler=false;
  /// @brief  puissance cumulée en Watt (remonté par l'enfant toute les 10 secondes)
  int puissance_cumul=0;
  /// @brief etat de la surchauffe
  int dallas_maitre=0;
  /// @brief sonde principale
  byte security=0;
  /// @brief  état ping
  bool ping=false;
  const char pingurl[35] = "ota.apper-solaire.org";
  int pingfail=0;
};

struct epoc {
public:
  int heure;
  int minute;
  int seconde;
  int jour;
  int mois;
  int annee;
  int weekday;
};




#endif