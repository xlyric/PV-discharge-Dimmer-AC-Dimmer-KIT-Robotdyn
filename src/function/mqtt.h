#ifndef MQTT_FUNCTIONS
#define MQTT_FUNCTIONS

// Web services
// #include <ESP8266WiFi.h>
// #include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <ESP8266HTTPClient.h> 
#include <ArduinoJson.h> 
#include "config/config.h"

#ifdef ESP32
// Web services
  #include "WiFi.h"
  #include <AsyncTCP.h>
  #include "HTTPClient.h"
#else
// Web services
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESP8266HTTPClient.h> 
#endif

extern Config config; 
extern System sysvar;
extern HTTPClient http;
extern WiFiClient domotic_client;

extern HA device_dimmer_child_mode;

extern HA device_dimmer_maxtemp;
extern HA device_dimmer_maxpow;
extern HA device_dimmer_minpow;
extern HA device_dimmer_starting_pow;
extern HA device_dimmer_maxtemp;
extern HA device_dimmer_on_off;
extern HA device_dimmer_alarm_temp;
extern HA device_dimmer_power;

extern bool discovery_temp; 
extern bool alerte; 
extern String logs; 
extern char buffer[1024];


  /// @brief Enregistrement du dimmer sur MQTT pour récuperer les informations remonté par MQTT
  /// @param Subscribedtopic 
  /// @param message 
  /// @param length 

  String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  // String node_ids = WiFi.macAddress().substring(0,2)+ WiFi.macAddress().substring(4,6)+ WiFi.macAddress().substring(8,10) + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17); // non utilisé
  String node_id = String("dimmer-") + node_mac; 
  String switch_command = String("homeassistant/switch/"+ node_id + "/command");
  String number_command = String("homeassistant/number/"+ node_id + "/command");
  String select_command = String("homeassistant/select/"+ node_id + "/command");
  String button_command = String("homeassistant/button/"+ node_id + "/command");
  String save_command = String("Xlyric/sauvegarde/"+ node_id );
  String topic = "homeassistant/sensor/"+ node_id +"/status";  

void callback(char* Subscribedtopic, byte* message, unsigned int length) {
  StaticJsonDocument<1024> doc2;
  deserializeJson(doc2, message);
  if (strcmp( Subscribedtopic, config.SubscribePV ) == 0 && doc2.containsKey("dimmer")) { 
    int puissancemqtt = doc2["dimmer"]; 
    puissancemqtt = puissancemqtt - config.startingpow;
    if (puissancemqtt < 0) puissancemqtt = 0;
    //if (puissancemqtt > config.maxpow) puissancemqtt = config.maxpow;
    if (sysvar.puissance != puissancemqtt ) {
      sysvar.puissance = puissancemqtt;
      logs += "MQTT power at " + String(sysvar.puissance) + "\r\n";
      sysvar.change=1; 
    }
    else {
      device_dimmer.send(String(sysvar.puissance));
      device_dimmer_power.send(String(sysvar.puissance*config.charge/100));
    }
  }
  if (strcmp( Subscribedtopic, config.SubscribeTEMP ) == 0  && doc2.containsKey("temperature")) { 
    float temperaturemqtt = doc2["temperature"]; 
    if (!discovery_temp) {
      discovery_temp = true;
      device_dimmer_alarm_temp.discovery();
      device_temp.discovery();
      device_dimmer_maxtemp.discovery();
      device_dimmer_alarm_temp.send(stringbool(false));
      device_dimmer_maxtemp.send(String(config.maxtemp));
    }
    device_temp.send(String(sysvar.celsius));



    if (sysvar.celsius != temperaturemqtt ) {
      sysvar.celsius = temperaturemqtt;
      logs += "MQTT temp at " + String(sysvar.celsius) + "\r\n";
    }
  }
    //   logs += "Subscribedtopic : " + String(Subscribedtopic)+ "\r\n";
    // logs += "switchcommand : " + String(switch_command)+ "\r\n";
//#ifdef  STANDALONE // désactivé sinon ne fonctionne pas avec ESP32
  if (strcmp( Subscribedtopic, switch_command.c_str() ) == 0) { 
    #ifdef RELAY1
      if (doc2.containsKey("relay1")) { 
          int relay = doc2["relay1"]; 
          if ( relay == 0) { digitalWrite(RELAY1 , LOW); }
          else { digitalWrite(RELAY1 , HIGH); } 
          logs += "RELAY1 at " + String(relay) + "\r\n"; 
          device_relay1.send(String(relay));
      }
    #endif
    #ifdef RELAY2
      if (doc2.containsKey("relay2")) { 
          int relay = doc2["relay2"]; 
          if ( relay == 0) { digitalWrite(RELAY2 , LOW); }
          else { digitalWrite(RELAY2 , HIGH); } 
          logs += "RELAY2 at " + String(relay) + "\r\n"; 
          device_relay2.send(String(relay));      
      }
    #endif
    if (doc2.containsKey("on_off")) { 
        config.dimmer_on_off = doc2["on_off"]; 
        logs += "Dimmer ON_OFF at " + String(config.dimmer_on_off) + "\r\n"; 
        device_dimmer_on_off.send(String(config.dimmer_on_off));      
        sysvar.change=1; 
    }
  } 
//#endif
  if (strcmp( Subscribedtopic, number_command.c_str() ) == 0) { 
    if (doc2.containsKey("starting_power")) { 
      int startingpow = doc2["starting_power"]; 
      if (config.startingpow != startingpow ) {
        config.startingpow = startingpow;
        logs += "MQTT starting_pow at " + String(startingpow) + "\r\n";
        device_dimmer_starting_pow.send(String(startingpow));
        sysvar.change=1; 
      }
    }
    else if (doc2.containsKey("minpow")) { 
      int minpow = doc2["minpow"]; 
      if (config.minpow != minpow ) {
        config.minpow = minpow;
        logs += "MQTT minpow at " + String(minpow) + "\r\n";
        device_dimmer_minpow.send(String(minpow));
        sysvar.change=1; 
      }
    }
    else if (doc2.containsKey("maxpow")) { 
      int maxpow = doc2["maxpow"]; 
      if (config.maxpow != maxpow ) {
        config.maxpow = maxpow;
        logs += "MQTT maxpow at " + String(maxpow) + "\r\n";
        device_dimmer_maxpow.send(String(maxpow));
        sysvar.change=1; 
      }
    }
    else if (doc2.containsKey("maxtemp")) { 
      int maxtemp = doc2["maxtemp"]; 
      if (config.maxtemp != maxtemp ) {
        config.maxtemp = maxtemp;
        logs += "MQTT maxtemp at " + String(maxtemp) + "\r\n";
        device_dimmer_maxtemp.send(String(maxtemp));
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
        device_dimmer_child_mode.send(String(config.mode));
        logs += "MQTT child mode at " + String(childmode) + "\r\n";
      }
    }
  }
  if (strcmp( Subscribedtopic, save_command.c_str() ) == 0) { 
        strlcpy(config.hostname , doc2["hostname"], sizeof(config.hostname));
        config.port = doc2["port"];
        strlcpy(config.Publish , doc2["Publish"], sizeof(config.Publish));
        config.IDXTemp = doc2["IDXTemp"];
        config.maxtemp = doc2["maxtemp"];
        config.IDXAlarme = doc2["IDXAlarme"];
        config.IDX = doc2["IDX"];  
        config.startingpow = doc2["startingpow"];
        config.minpow = doc2["minpow"];
        config.maxpow = doc2["maxpow"];
        strlcpy(config.child , doc2["child"], sizeof(config.child)) ;
        strlcpy(config.mode , doc2["mode"], sizeof(config.mode)) ;
        strlcpy(config.SubscribePV , doc2["SubscribePV"], sizeof(config.SubscribePV));
        strlcpy(config.SubscribeTEMP , doc2["SubscribeTEMP"], sizeof(config.SubscribeTEMP));
        //saveConfiguration(filename_conf, config);  
        Serial.println("sauvegarde conf mqtt ");
        serializeJson(doc2, buffer);
        Serial.println(config.hostname);
        Serial.println(buffer);
        
      }


}






//// envoie de commande MQTT 
void mqtt(String idx, String value)
{

  if (idx != "0") { // Autant vérifier qu'une seule fois?
    
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
    
    if (mqtt_config.domoticz){
      String nvalue = "0" ; 
      if ( value != "0" ) { nvalue = "2" ; }
      String message = "  { \"idx\" : " + idx +" ,   \"svalue\" : \"" + value + "\",  \"nvalue\" : " + nvalue + "  } ";
      //client.loop();
      client.publish(config.Publish, String(message).c_str(), true);   
    }

    if (mqtt_config.jeedom){
      String jdompub = String(config.Publish) + "/"+idx ;
      client.publish(jdompub.c_str() , value.c_str(), true);
      //client.loop();
    }
    Serial.println("MQTT SENT");
  }
}


//// communication avec carte fille ( HTTP )

void child_communication(int delest_power, bool equal = false){
  int instant_power ;
  String baseurl; 
  if (config.dispo != 0 ) { if (equal) { instant_power = config.dispo/2 ;} else {instant_power = config.dispo; } }
  else { instant_power = delest_power*config.charge/100   ; }  /// ça posera problème si il y a pas de commandes de puissance en W comme le 2eme dimmer se calque sur la puissance du 1er 

  baseurl = "/?POWER=" + String(delest_power) + "&puissance=" + instant_power ; http.begin(domotic_client,config.child,80,baseurl); 
  http.GET();
  http.end(); 
  logs += "child at " + String(delest_power) + " " + String(instant_power) +  "\r\n";
}


//////////// reconnexion MQTT

void reconnect() {
  // String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  // String node_ids = WiFi.macAddress().substring(0,2)+ WiFi.macAddress().substring(4,6)+ WiFi.macAddress().substring(8,10) + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  // String node_id = String("dimmer-") + node_mac; 
  // String topic = "homeassistant/sensor/"+ node_id +"/status";  
  
  // Loop until we're reconnected
  int timeout = 0; 
  if  (LittleFS.exists("/mqtt.json"))
  {
    while (!client.connected()) {
      
      Serial.print("Attempting MQTT connection...");
      logs += loguptime() + " Reconnect MQTT\r\n";
      // Create a random client ID
      // String clientId = "Dimmer";
      // clientId += String(random(0xffff), HEX);
      // Attempt to connect
      //node_id.c_str(),mqtt_config.username, mqtt_config.password, topic.c_str(), 2, true, "offline"
    //  if (client.connect(node_id.c_str(),mqtt_config.username, mqtt_config.password, topic.c_str(), 2, true, "offline")) {       //Connect to MQTT server
     if (client.connect(node_id.c_str(),mqtt_config.username, mqtt_config.password)) {
        client.publish(String(topic).c_str() , "online", true); // status Online
        Serial.println("connected");
        logs += "Connected\r\n";
        if (mqtt_config.mqtt && strlen(config.SubscribePV) !=0 ) {client.subscribe(config.SubscribePV);}
        if (mqtt_config.mqtt && strlen(config.SubscribeTEMP) != 0 ) {client.subscribe(config.SubscribeTEMP);}
        client.subscribe(switch_command.c_str());
        client.subscribe(number_command.c_str());
        client.subscribe(select_command.c_str());
        client.subscribe(button_command.c_str());
                
        String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
        String node_id = String("dimmer-") + node_mac; 
        String save_command = String("Xlyric/sauvegarde/"+ node_id );
        //client.subscribe(save_command.c_str());
         int instant_power = dimmer.getPower();
        mqtt(String(config.IDX), String(String(instant_power)));   /// correction 19/04 valeur remonté au dessus du max conf
        device_dimmer.send(String(instant_power)); 
        device_dimmer_power.send(String(instant_power * config.charge/100)); 
      } else {
        Serial.print("failed, rc=");
        logs += loguptime() + "Fail and retry\r\n";
        Serial.print(client.state());
        Serial.println(" try again in 2 seconds");
        // Wait 2 seconds before retrying
        delay(2000);   // 24/01/2023 passage de 5 a 2s
        timeout++; // after 10s break for apply command 
        if (timeout > 5) {
            Serial.println(" try again next time ") ; 
            logs += loguptime() +"retry later\r\n";
            break;
            }

      }
    }
  } else {  Serial.println(" Filesystem not present "); delay(5000); }
}
#endif