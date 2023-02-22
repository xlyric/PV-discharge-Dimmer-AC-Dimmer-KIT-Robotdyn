#ifndef MQTT_FUNCTIONS
#define MQTT_FUNCTIONS

// Web services
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266HTTPClient.h> 
#include <ArduinoJson.h> 
#include "config/config.h"

extern Config config; 
extern System sysvar;
extern HTTPClient http;
extern WiFiClient domotic_client;

extern HA device_dimmer_child_mode;

extern HA device_dimmer_maxtemp;
extern HA device_dimmer_maxpow;
extern HA device_dimmer_minpow;
extern HA device_starting_pow;
extern HA device_dimmer_maxtemp;
extern HA device_dimmer_on_off;
extern HA device_dimmer_alarm_temp;

extern bool discovery_temp; 
extern bool alerte; 



/// @brief Enregistrement du dimmer sur MQTT pour récuperer les informations remonté par MQTT
/// @param Subscribedtopic 
/// @param message 
/// @param length 
void callback(char* Subscribedtopic, byte* message, unsigned int length) {
  String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  String node_ids = WiFi.macAddress().substring(0,2)+ WiFi.macAddress().substring(4,6)+ WiFi.macAddress().substring(8,10) + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  String node_id = String("dimmer-") + node_mac; 
  String switch_command = String("homeassistant/switch/"+ node_id + "/command");
  String number_command = String("homeassistant/number/"+ node_id + "/command");
  String select_command = String("homeassistant/select/"+ node_id + "/command");
  String button_command = String("homeassistant/button/"+ node_id + "/command");

  StaticJsonDocument<64> doc2;
  deserializeJson(doc2, message);
  if (strcmp( Subscribedtopic, config.SubscribePV ) == 0 && doc2.containsKey("dimmer")) { 
    int puissancemqtt = doc2["dimmer"]; 
    puissancemqtt = puissancemqtt - config.startingpow;
    if (puissancemqtt < 0) puissancemqtt = 0;
    // if (puissancemqtt > config.maxpow) puissancemqtt = config.maxpow;
    if (sysvar.puissance != puissancemqtt ) {
      sysvar.puissance = puissancemqtt;
      logs += "MQTT power at " + String(sysvar.puissance) + "\r\n";
      sysvar.change=1; 
    }
    else {
      device_dimmer.send(String(sysvar.puissance),false);
    }
  }
  if (strcmp( Subscribedtopic, config.SubscribeTEMP ) == 0  && doc2.containsKey("temperature")) { 
    float temperaturemqtt = doc2["temperature"]; 
    if (!discovery_temp) {
      discovery_temp = true;
      device_dimmer_alarm_temp.discovery();
      device_temp.discovery();
      device_dimmer_maxtemp.discovery();
    }
    device_temp.send(String(sysvar.celsius),false);
    device_dimmer_alarm_temp.send(stringbool(alerte),false);
    device_dimmer_maxtemp.send(String(config.maxtemp),false);


    if (sysvar.celsius != temperaturemqtt ) {
      sysvar.celsius = temperaturemqtt;
      logs += "MQTT temp at " + String(sysvar.celsius) + "\r\n";
    }
  }
    //   logs += "Subscribedtopic : " + String(Subscribedtopic)+ "\r\n";
    // logs += "switchcommand : " + String(switch_command)+ "\r\n";

  if (strcmp( Subscribedtopic, switch_command.c_str() ) == 0) { 
    if (doc2.containsKey("relay1")) { 
        int relay = doc2["relay1"]; 
        if ( relay == 0) { digitalWrite(RELAY1 , LOW); }
        else { digitalWrite(RELAY1 , HIGH); } 
        logs += "RELAY1 at " + String(relay) + "\r\n"; 
        device_relay1.send(String(relay),false);
    }
    if (doc2.containsKey("relay2")) { 
        int relay = doc2["relay2"]; 
        if ( relay == 0) { digitalWrite(RELAY2 , LOW); }
        else { digitalWrite(RELAY2 , HIGH); } 
        logs += "RELAY2 at " + String(relay) + "\r\n"; 
        device_relay2.send(String(relay),false);      
    }
    if (doc2.containsKey("on_off")) { 
        config.dimmer_on_off = doc2["on_off"]; 
        logs += "Dimmer ON_OFF at " + String(config.dimmer_on_off) + "\r\n"; 
        device_dimmer_on_off.send(String(config.dimmer_on_off),false);      
        sysvar.change=1; 
    }
  } 
  if (strcmp( Subscribedtopic, number_command.c_str() ) == 0) { 
    if (doc2.containsKey("starting_power")) { 
      int startingpow = doc2["starting_power"]; 
      if (config.startingpow != startingpow ) {
        config.startingpow = startingpow;
        logs += "MQTT starting_pow at " + String(startingpow) + "\r\n";
        device_starting_pow.send(String(startingpow),false);
        sysvar.change=1; 
      }
    }
    else if (doc2.containsKey("minpow")) { 
      int minpow = doc2["minpow"]; 
      if (config.minpow != minpow ) {
        config.minpow = minpow;
        logs += "MQTT minpow at " + String(minpow) + "\r\n";
        device_dimmer_minpow.send(String(minpow),false);
        sysvar.change=1; 
      }
    }
    else if (doc2.containsKey("maxpow")) { 
      int maxpow = doc2["maxpow"]; 
      if (config.maxpow != maxpow ) {
        config.maxpow = maxpow;
        logs += "MQTT maxpow at " + String(maxpow) + "\r\n";
        device_dimmer_maxpow.send(String(maxpow),false);
        sysvar.change=1; 
      }
    }
    else if (doc2.containsKey("maxtemp")) { 
      int maxtemp = doc2["maxtemp"]; 
      if (config.maxtemp != maxtemp ) {
        config.maxtemp = maxtemp;
        logs += "MQTT maxtemp at " + String(maxtemp) + "\r\n";
        device_dimmer_maxtemp.send(String(maxtemp),false);
        sysvar.change=1; 
      }
    }
  }
//save
  if (strcmp( Subscribedtopic, button_command.c_str() ) == 0) { 
    if (doc2.containsKey("save")) { 
      if (doc2["save"] == "1" ) {
        logs += "MQTT save command \r\n";
        saveConfiguration(filename_conf, config);  
      }
    }
  }
//child mode
  if (strcmp( Subscribedtopic, select_command.c_str() ) == 0) { 
    if (doc2.containsKey("child_mode")) { 
      String childmode = doc2["child_mode"]; 
      if (config.mode != doc2["child_mode"] ) {
        strlcpy(config.mode, doc2["child_mode"], sizeof(config.mode));
        device_dimmer_child_mode.send(String(config.mode),false);
        logs += "MQTT child mode at " + String(childmode) + "\r\n";
      }
    }
  }

}






//// envoie de commande MQTT 
void mqtt(String idx, String value)
{

  if (idx != 0) { // Autant vérifier qu'une seule fois?
    
  // Grace a l'ajout de "exp_aft" sur le discovery, 
  // je préfère envoyer power et temp séparément, à chaque changement de valeur.
  // MQTT_INTERVAL à affiner, mais OK selon mes tests.
  // Si pas de valeur publiée dans ce délai, la valeur sur HA passe en indisponible.
  // Permet de détecter un problème

    // StaticJsonDocument<256> infojson;
    // infojson["power"] = String(puissance);
    // infojson["temperature"] = String(celsius);
    // char json_string[256];
    // serializeJson(infojson, json_string);
    // device_dimmer.send2(json_string);
    String nvalue = "0" ; 
    if ( value != "0" ) { nvalue = "2" ; }
    String message = "  { \"idx\" : " + idx +" ,   \"svalue\" : \"" + value + "\",  \"nvalue\" : " + nvalue + "  } ";


    client.loop();
    client.publish(config.Publish, String(message).c_str(), true);      

    String jdompub = String(config.Publish) + "/"+idx ;
    client.publish(jdompub.c_str() , value.c_str(), true);

    Serial.println("MQTT SENT");
  }
}


//// communication avec carte fille

void child_communication(int delest_power){

  String baseurl; 
  baseurl = "/?POWER=" + String(delest_power) ; http.begin(domotic_client,config.child,80,baseurl); 
  http.GET();
  http.end(); 
  logs += "child at " + String(delest_power) + "\r\n";
}


//////////// reconnexion MQTT

void reconnect() {
  String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  String node_ids = WiFi.macAddress().substring(0,2)+ WiFi.macAddress().substring(4,6)+ WiFi.macAddress().substring(8,10) + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  String node_id = String("dimmer-") + node_mac; 
  String topic = "homeassistant/sensor/"+ node_id +"/status";  

  // Loop until we're reconnected
  int timeout = 0; 
  if  (LittleFS.exists("/mqtt.json"))
  {
    while (!client.connected()) {
      
      Serial.print("Attempting MQTT connection...");
      logs += "Reconnect MQTT\r\n";
      // Create a random client ID
      // String clientId = "Dimmer";
      // clientId += String(random(0xffff), HEX);
      // Attempt to connect
      if (client.connect(node_id.c_str(),mqtt_config.username, mqtt_config.password, topic.c_str(), 2, true, "offline")) {       //Connect to MQTT server
    // if (client.connect(clientId.c_str(),mqtt_config.username, mqtt_config.password)) {
        Serial.println("connected");
        logs += "Connected\r\n";
        if (strcmp( config.SubscribePV, "none") != 0 ) {client.subscribe(config.SubscribePV);}
        if (strcmp( config.SubscribeTEMP, "none") != 0 ) {client.subscribe(config.SubscribeTEMP);}
        client.publish(String(topic).c_str() , "online", true); // status Online


      } else {
        Serial.print("failed, rc=");
        logs += "Fail and retry\r\n";
        Serial.print(client.state());
        Serial.println(" try again in 2 seconds");
        // Wait 2 seconds before retrying
        delay(2000);   // 24/01/2023 passage de 5 a 2s
        timeout++; // after 10s break for apply command 
        if (timeout > 5) {
            Serial.println(" try again next time ") ; 
            logs += "retry later\r\n";
            break;
            }

      }
    }
  } else {  Serial.println(" Filesystem not present "); delay(5000); }
   
}
#endif