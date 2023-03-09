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

extern MQTT device_dimmer_child_mode;

extern MQTT device_dimmer_maxtemp;
extern MQTT device_dimmer_maxpow;
extern MQTT device_dimmer_minpow;
extern MQTT device_dimmer_starting_pow;
extern MQTT device_dimmer_maxtemp;
extern MQTT device_dimmer_on_off;
extern MQTT device_dimmer_alarm_temp;

extern bool discovery_temp; 
extern bool alerte; 
extern String logs; 
extern char buffer[1024];

String stringboolMQTT(bool mybool);

  /// @brief Enregistrement du dimmer sur MQTT pour récuperer les informations remonté par MQTT
  /// @param Subscribedtopic 
  /// @param message 
  /// @param length 



  String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  // String node_ids = WiFi.macAddress().substring(0,2)+ WiFi.macAddress().substring(4,6)+ WiFi.macAddress().substring(8,10) + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  String node_id = String("Dimmer-") + node_mac; 
  String topic = "homeassistant/sensor/"+ node_id +"/status";  
  String topic_Xlyric = "Xlyric/"+ node_id +"/";

  String command_switch = String(topic_Xlyric + "switch/command");
  String command_number = String(topic_Xlyric + "number/command");
  String command_select = String(topic_Xlyric + "select/command");
  String command_button = String(topic_Xlyric + "button/command");
  String command_save = String("Xlyric/sauvegarde/"+ node_id );

void callback(char* Subscribedtopic, byte* message, unsigned int length) {
  StaticJsonDocument<1024> doc2;
  deserializeJson(doc2, message);
  if (strcmp( Subscribedtopic, config.SubscribePV ) == 0 && doc2.containsKey("dimmer") && config.dimmer_on_off == 1) { 
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
      device_dimmer.send(String(sysvar.puissance));
    }
  }
  if (strcmp( Subscribedtopic, config.SubscribeTEMP ) == 0  && doc2.containsKey("temperature")) { 
    float temperaturemqtt = doc2["temperature"]; 
    if (!discovery_temp) {
      discovery_temp = true;
      device_dimmer_alarm_temp.HA_discovery();
      device_temp.HA_discovery();
      device_dimmer_maxtemp.HA_discovery();
      device_dimmer_alarm_temp.send(stringboolMQTT(false));
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
  if (strcmp( Subscribedtopic, command_switch.c_str() ) == 0) { 
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
  if (strcmp( Subscribedtopic, command_number.c_str() ) == 0) { 
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
  if (strcmp( Subscribedtopic, command_button.c_str() ) == 0) { 
    if (doc2.containsKey("save")) { 
      if (doc2["save"] == "1" ) {
        logs += "MQTT save command \r\n";
        saveConfiguration(filename_conf, config);  
      }
    }
  }
//child mode
  if (strcmp( Subscribedtopic, command_select.c_str() ) == 0) { 
    if (doc2.containsKey("child_mode")) { 
      String childmode = doc2["child_mode"]; 
      if (config.mode != doc2["child_mode"] ) {
        strlcpy(config.mode, doc2["child_mode"], sizeof(config.mode));
        device_dimmer_child_mode.send(String(config.mode));
        logs += "MQTT child mode at " + String(childmode) + "\r\n";
      }
    }
  }
  if (strcmp( Subscribedtopic, command_save.c_str() ) == 0) { 
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
void Mqtt_send_DOMOTICZ(String idx, String value)
{
  if (config.DOMOTICZ) {
    String nvalue = "0" ; 
    if ( value != "0" ) { nvalue = "2" ; }
    String message = "  { \"idx\" : " + idx +" ,   \"svalue\" : \"" + value + "\",  \"nvalue\" : " + nvalue + "  } ";


    client.loop();
    // delay(10);  // <- fixes some issues with WiFi stability

    client.publish(config.Publish, String(message).c_str(), true);      
    yield();
    // String jdompub = String(config.Publish) + "/"+idx ;
    // client.publish(jdompub.c_str() , value.c_str(), true);

    Serial.println("MQTT SENT");
  }
}


//// communication avec carte fille ( HTTP )

void child_communication(int delest_power){

  String baseurl; 
  baseurl = "/?POWER=" + String(delest_power) ; http.begin(domotic_client,config.child,80,baseurl); 
  http.GET();
  http.end(); 
  logs += "child at " + String(delest_power) + "\r\n";
}


//////////// reconnexion MQTT

void reconnect() {
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
      if (client.connect(node_id.c_str(),mqtt_config.username, mqtt_config.password, String(topic_Xlyric +"status").c_str(), 2, true, "offline")) {       //Connect to MQTT server
    // if (client.connect(clientId.c_str(),mqtt_config.username, mqtt_config.password)) {
        Serial.println("connected");
        logs += "Connected\r\n";
        if (strcmp(config.PVROUTER, "mqtt") == 0 && config.SubscribePV != NULL ) {client.subscribe(config.SubscribePV);}
        if (strcmp(config.PVROUTER, "mqtt") == 0 && config.SubscribeTEMP != NULL ) {client.subscribe(config.SubscribeTEMP);}
        client.subscribe(command_button.c_str());
        client.subscribe(command_number.c_str());
        client.subscribe(command_select.c_str());
        client.subscribe(command_switch.c_str());

        client.publish(String(topic_Xlyric +"status").c_str() , "online", true); // status Online
        yield();
        
        String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
        String node_id = String("Dimmer-") + node_mac; 
        String save_command = String("Xlyric/sauvegarde/"+ node_id );
        //client.subscribe(save_command.c_str());

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

String stringboolMQTT(bool mybool){
  String truefalse = "true";
  if (mybool == false ) {truefalse = "false";}
  return String(truefalse);
  }
#endif