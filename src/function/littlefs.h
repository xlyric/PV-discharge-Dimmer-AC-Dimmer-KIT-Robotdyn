#ifndef LITTLEFS_FUNCTIONS
#define LITTLEFS_FUNCTIONS

#include "config/enums.h"

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson
#include "function/ha.h"

#include "uptime.h"

#if defined(ESP32) || defined(ESP32ETH)
  #include <FS.h>
  #include "SPIFFS.h"
  #define LittleFS SPIFFS // Fonctionne, mais est-ce correct? 
#else
  #include <LittleFS.h> // NOSONAR
#endif


constexpr const char *wifi_conf = "/wifi.json";
constexpr const char *programme_conf = "/programme.json";


extern Config config;  //NOSONAR

extern Logs logging; 

extern String node_id; 
extern Wifi_struct  wifi_config_fixe; //NOSONAR 

//flag for saving data
extern bool shouldSaveConfig ;  //NOSONAR

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

String loguptime(String log) {
  String uptime_stamp;
  uptime::calculateUptime();
  uptime_stamp = String(uptime::getDays())+":"+String(uptime::getHours())+":"+String(uptime::getMinutes())+":"+String(uptime::getSeconds())+ "\t"+log +"\r\n";
  return uptime_stamp;
}


//***********************************
//************* Gestion de la configuration - Lecture du fichier de configuration
//***********************************

//////////////wifi ip fixe 

bool loadwifiIP(const char *wifi_conf, Wifi_struct &wifi_config_fixe) {
  // Open file for reading
  File configFile = LittleFS.open(wifi_conf, "r");

      // Allocate a temporary JsonDocument
      // Don't forget to change the capacity to match your requirements.
      // Use arduinojson.org/v6/assistant to compute the capacity.
      JsonDocument doc;

      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, configFile);
      if (error) {
        Serial.println(F("Failed to read wifi config"));
      }

      
      // Copy values from the JsonDocument to the Config
      
      strlcpy(wifi_config_fixe.static_ip,                  // <- destination
              doc["IP"] | "", // <- source
              sizeof(wifi_config_fixe.static_ip));         // <- destination's capacity
      
      strlcpy(wifi_config_fixe.static_sn,                  // <- destination
              doc["mask"] | "255.255.255.0", // <- source
              sizeof(wifi_config_fixe.static_sn));         // <- destination's capacity

      strlcpy(wifi_config_fixe.static_gw,                  // <- destination
              doc["gateway"] | "192.168.1.254", // <- source
              sizeof(wifi_config_fixe.static_gw));         // <- destination's capacity

      configFile.close();

return true;    
}

void savewifiIP(const char *wifi_conf, Wifi_struct &wifi_config_fixe) {
  
  // Open file for writing
   File configFile = LittleFS.open(wifi_conf, "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing in function Save configuration"));
    return;
  } 

    // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  JsonDocument doc;

  // Set the values in the document
  doc["IP"] = wifi_config_fixe.static_ip;
  doc["mask"] = wifi_config_fixe.static_sn;
  doc["gateway"] = wifi_config_fixe.static_gw;

  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file in function Save configuration "));
    logging.Set_log_init(Failed_write_wifi_config);
  }

  // Close the file
  configFile.close();
}


bool test_fs_version() {

  // Open file for reading

  File file = LittleFS.open("/version", "r");
  if (!file) {
    logging.Set_log_init(FS_version_is_missing);
    return false;
  }
   // comparaison entre le contenu du fichier et la version du code FS_RELEASE
  String version = file.readStringUntil('\n');
  file.close(); 
  if (version.toInt() < String(FS_RELEASE).toInt() )  {
    logging.Set_log_init(FS_version_is_not_same);
    return false;
  }
  logging.Set_log_init(FS_version_is_same);
  return true;
}

#endif