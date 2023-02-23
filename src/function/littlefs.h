#ifndef LITTLEFS_FUNCTIONS
#define LITTLEFS_FUNCTIONS

#include "config/enums.h"

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson
#include "function/ha.h"


#ifdef ESP32
  #include <FS.h>
  #include "SPIFFS.h"
  #define LittleFS SPIFFS // Fonctionne, mais est-ce correct? 
#else
  #include <LittleFS.h>
#endif

const char *mqtt_conf = "/mqtt.json";
const char *filename_conf = "/config.json";
extern Config config; 
extern String loginit;
extern String logs; 


String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
String node_id = String("sauvegarde/dimmer-") + node_mac;

// Loads the configuration from a file
void loadConfiguration(const char *filename, Config &config) {
  // Open file for reading
  File configFile = LittleFS.open(filename_conf, "r");

   // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<2048> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println(F("Failed to read file, using default configuration"));
    loginit +="Failed to read file config File, use default\r\n"; 
    }
  // Copy values from the JsonDocument to the Config
  
  strlcpy(config.hostname,                  // <- destination
          doc["hostname"] | "192.168.1.22", // <- source
          sizeof(config.hostname));         // <- destination's capacity
  config.port = doc["port"] | 1883;
  strlcpy(config.Publish,                 
          doc["Publish"] | "domoticz/in", 
          sizeof(config.Publish));        
  config.IDXTemp = doc["IDXTemp"] | 200; 
  config.maxtemp = doc["maxtemp"] | 60; 
  config.IDXAlarme = doc["IDXAlarme"] | 202; 
  config.IDX = doc["IDX"] | 201; 
  config.startingpow = doc["startingpow"] | 0; 
  config.minpow = doc["minpow"] | 5;
  config.maxpow = doc["maxpow"] | 50; 
  strlcpy(config.child,                  
          doc["child"] | "192.168.1.20", 
          sizeof(config.child));         
  strlcpy(config.mode,                  
          doc["mode"] | "off", 
          sizeof(config.mode));
  strlcpy(config.SubscribePV,                 
        doc["SubscribePV"] | "homeassistant/sensor/PvRouter-xxxx/statedimmer", 
        sizeof(config.SubscribePV));    
  strlcpy(config.SubscribeTEMP,                 
        doc["SubscribeTEMP"] | "homeassistant/sensor/dimmer-xxxx/state", 
        sizeof(config.SubscribeTEMP));
  config.dimmer_on_off = doc["dimmer_on_off"] | 1; 

  configFile.close();
  
      
}

//***********************************
//************* Gestion de la configuration - sauvegarde du fichier de configuration
//***********************************

void saveConfiguration(const char *filename, const Config &config) {
  
  // Open file for writing
   File configFile = LittleFS.open(filename_conf, "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing"));
    logs +="Failed to read file config File, use default\r\n"; 
  
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<1024> doc;

  // Set the values in the document
  doc["hostname"] = config.hostname;
  doc["port"] = config.port;
  doc["Publish"] = config.Publish;
  doc["IDXTemp"] = config.IDXTemp;
  doc["maxtemp"] = config.maxtemp;
  doc["IDXAlarme"] = config.IDXAlarme;
  doc["IDX"] = config.IDX;  
  doc["startingpow"] = config.startingpow;
  doc["minpow"] = config.minpow;
  doc["maxpow"] = config.maxpow;
  doc["child"] = config.child;
  doc["mode"] = config.mode;
  doc["SubscribePV"] = config.SubscribePV;
  doc["SubscribeTEMP"] = config.SubscribeTEMP;
  doc["dimmer_on_off"] = config.dimmer_on_off;

  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  
  char buffer[1024];
  size_t n = serializeJson(doc, buffer);
  client.publish((node_id).c_str() ,buffer,  true);

  // Close the file
  configFile.close();
}

//***********************************
//************* Gestion de la configuration - Lecture du fichier de configuration
//***********************************

//***********************************
//************* Gestion de la configuration - Lecture du fichier de configuration
//***********************************

bool loadmqtt(const char *filename, Mqtt &mqtt_config) {
  // Open file for reading
  File configFile = LittleFS.open(mqtt_conf, "r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println(F("Failed to read MQTT config "));
    return false;
  }

  
  // Copy values from the JsonDocument to the Config
  
  strlcpy(mqtt_config.username,                  // <- destination
          doc["MQTT_USER"] | "", // <- source
          sizeof(mqtt_config.username));         // <- destination's capacity
  
  strlcpy(mqtt_config.password,                  // <- destination
          doc["MQTT_PASSWORD"] | "", // <- source
          sizeof(mqtt_config.password));         // <- destination's capacity
  mqtt_config.mqtt = doc["mqtt"] | true;
  configFile.close();

return true;    
}

void savemqtt(const char *filename, const Mqtt &mqtt_config) {
  
  // Open file for writing
   File configFile = LittleFS.open(mqtt_conf, "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing in function Save configuration"));
    return;
  } 

    // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<1024> doc;

  // Set the values in the document
  doc["MQTT_USER"] = mqtt_config.username;
  doc["MQTT_PASSWORD"] = mqtt_config.password;
  doc["mqtt"] = mqtt_config.mqtt;
  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file in function Save configuration "));
    logs += "Failed to write MQTT config\r\n";
  }

  // Close the file
  configFile.close();
}

#endif