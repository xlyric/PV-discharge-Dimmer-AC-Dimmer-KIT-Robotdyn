#ifndef MQTT_FUNCTIONS
#define MQTT_FUNCTIONS

// Web services
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h> 
#include "config/config.h"
#include "function/unified_dimmer.h"

#if defined(ESP32) || defined(ESP32ETH)
// Web services
  #include "WiFi.h"
  #include "HTTPClient.h"
#else
// Web services
  #include <ESP8266WiFi.h>
  //#include <ESPAsyncTCP.h>
  #include <ESP8266HTTPClient.h> 
#endif

extern Config config; 
extern System sysvar;
extern HTTPClient http;
extern WiFiClient domotic_client;
extern bool mqttConnected;
extern HA device_dimmer_child_mode;

extern HA device_dimmer;
extern HA device_dimmer_maxtemp;
extern HA device_dimmer_maxpow;
extern HA device_dimmer_minpow;
extern HA device_dimmer_starting_pow;
extern HA device_dimmer_maxtemp;
extern HA device_dimmer_on_off;
extern HA device_dimmer_alarm_temp;
extern HA device_dimmer_power;
extern HA device_dimmer_send_power; 
extern HA device_dimmer_total_power;
extern HA device_temp[MAX_DALLAS];
extern HA device_temp_master;
extern HA device_relay1;
extern HA device_relay2;
extern HA device_cooler;
extern HA device_dimmer_alarm_temp_clear;


extern bool HA_reconnected;
extern bool discovery_temp; 
extern bool alerte; 
extern byte security; // sécurité
extern Logs logging; 
extern String devAddrNames[MAX_DALLAS];
extern   PubSubClient client; 


void connectToMqtt();
void onMqttConnect(bool sessionPresent);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void recreate_topic();

char buffer[1024];// NOSONAR
  /// @brief Enregistrement du dimmer sur MQTT pour récuperer les informations remonté par MQTT
  /// @param topic 
  /// @param message 
  /// @param length 

String stringBoolMQTT(bool mybool);

  String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  String node_id = String("dimmer-") + node_mac; 
  
  String topic_Xlyric = "Xlyric/" + String(config.say_my_name) +"/";
  
  String command_switch = String(topic_Xlyric + "command/switch");
  String command_number = String(topic_Xlyric + "command/number");
  String command_select = String(topic_Xlyric + "command/select");
  String command_button = String(topic_Xlyric + "command/button");
  const String HA_status = String("homeassistant/status");
  String command_save = String("Xlyric/sauvegarde/"+ node_id );

void callback(char* topic, byte* payload, unsigned int length) {
  char arrivage[length+1]; // Ajout d'un espace pour le caractère nul // NOSONAR
  int recup = 0;
  
    for (int i=0;i<length;i++) {
      arrivage[i] = (char)payload[i];
    }
   arrivage[length] = '\0'; // Ajouter le caractère nul à la fin

  // debug
  Serial.println("topic : " + String(topic));
  Serial.println("payload : " + String(arrivage));
  String fixedpayload = ((String)arrivage).substring(0,length);
  StaticJsonDocument<1152> doc2;
  deserializeJson(doc2, arrivage);
  /// @brief Enregistrement du dimmer sur MQTT pour récuperer les informations remontées par MQTT
  if (strcmp( topic, config.SubscribePV ) == 0 && doc2.containsKey("power")) { 
    int puissancemqtt = doc2["power"]; 
    puissancemqtt = puissancemqtt - config.startingpow;
    if (puissancemqtt < 0) puissancemqtt = 0;
    if (sysvar.puissance != puissancemqtt ) {
      sysvar.puissance = puissancemqtt;
      sysvar.change=1; 
    }
    else if (config.dimmer_on_off == 1){
      device_dimmer.send(String(sysvar.puissance));
      device_dimmer_send_power.send(String(sysvar.puissance));
      device_dimmer_power.send(String(sysvar.puissance*config.charge/100));
    }
  }
  /// @brief Enregistrement temperature
  if (strcmp( topic, config.SubscribeTEMP ) == 0 ){ 
    float temperaturemqtt = doc2[0]; 
    sysvar.dallas_maitre= deviceCount+1;
    devAddrNames[deviceCount+1] = "MQTT";
    if (!discovery_temp) {
      discovery_temp = true;
      device_dimmer_alarm_temp.HA_discovery();
      device_temp[sysvar.dallas_maitre].HA_discovery();
      device_temp_master.HA_discovery();
      device_dimmer_maxtemp.HA_discovery();
      device_dimmer_alarm_temp.send(stringBoolMQTT(sysvar.security));
      device_dimmer_maxtemp.send(String(config.maxtemp));
      device_dimmer_alarm_temp_clear.HA_discovery();
    }
    device_temp[sysvar.dallas_maitre].send(String(sysvar.celsius[sysvar.dallas_maitre]));
    Serial.println(sysvar.celsius[sysvar.dallas_maitre]);
    device_temp_master.send(String(sysvar.celsius[sysvar.dallas_maitre]));
    if (sysvar.celsius[sysvar.dallas_maitre] != temperaturemqtt ) {
      sysvar.celsius[sysvar.dallas_maitre] = temperaturemqtt;
      logging.Set_log_init("MQTT temp at ");
      logging.Set_log_init(String(sysvar.celsius[sysvar.dallas_maitre]));
      logging.Set_log_init("°C\r\n");
    }
  }

  /// @brief Enregistrement des requetes de commandes 
  if (strstr( topic, command_switch.c_str() ) != NULL) { 
    #ifdef RELAY1
      if (doc2.containsKey("relay1")) { 
          int relay = doc2["relay1"]; 
          if ( relay == 0) { digitalWrite(RELAY1 , LOW); }
          else { digitalWrite(RELAY1 , HIGH); } 
          logging.Set_log_init("RELAY1 at ");
          logging.Set_log_init(String(relay).c_str());
          logging.Set_log_init("\r\n"); 
          device_relay1.send(String(relay));
      }
    #endif
    #ifdef RELAY2
      if (doc2.containsKey("relay2")) { 
          int relay = doc2["relay2"]; 
          if ( relay == 0) { digitalWrite(RELAY2 , LOW); }
          else { digitalWrite(RELAY2 , HIGH); } 
          logging.Set_log_init("RELAY2 at ");
          logging.Set_log_init(String(relay).c_str());
          logging.Set_log_init("\r\n"); 
          device_relay2.send(String(relay));      
      }
    #endif
    if (doc2.containsKey("on_off")) { 
        int relay = doc2["on_off"]; 
        if ( relay == 0) { config.dimmer_on_off = false; }
          else { config.dimmer_on_off = true; }
        logging.Set_log_init("Dimmer ON_OFF at ");
        logging.Set_log_init(String(config.dimmer_on_off).c_str());
        logging.Set_log_init("\r\n"); 
        device_dimmer_on_off.send(String(config.dimmer_on_off));      
        sysvar.change=1; 
        
    }
    
  } 

  if (strstr( topic, command_number.c_str() ) != NULL) { 
    if (doc2.containsKey("starting_power")) { 
      int startingpow = doc2["starting_power"]; 
      if (config.startingpow != startingpow ) {
        config.startingpow = startingpow;
        logging.Set_log_init("MQTT starting_pow at ");
        logging.Set_log_init(String(startingpow).c_str());
        logging.Set_log_init("%\r\n");
        device_dimmer_starting_pow.send(String(startingpow));
        sysvar.change=1; 
      }
    }
    else if (doc2.containsKey("minpow")) { 
      int minpow = doc2["minpow"]; 
      if (config.minpow != minpow ) {
        config.minpow = minpow;
        logging.Set_log_init("MQTT minpow at " );
        logging.Set_log_init(String(minpow).c_str()); 
        logging.Set_log_init("%\r\n");
        device_dimmer_minpow.send(String(minpow));
        sysvar.change=1; 
      }
    }
    else if (doc2.containsKey("maxpow")) { 
      int maxpow = doc2["maxpow"]; 
      if (config.maxpow != maxpow ) {
        config.maxpow = maxpow;
        logging.Set_log_init("MQTT maxpow at ");
        logging.Set_log_init(String(maxpow).c_str());
        logging.Set_log_init("%\r\n");
        device_dimmer_maxpow.send(String(maxpow));
        sysvar.change=1; 
      }
    }
    else if (doc2.containsKey("powdimmer")) { 
      int powdimmer = doc2["powdimmer"]; 
      if (sysvar.puissance != powdimmer ) {
        if ( config.maxpow != 0 && powdimmer > config.maxpow ) { powdimmer = config.maxpow; } 
        sysvar.puissance = powdimmer;
        sysvar.change=1; 
        logging.Set_log_init("MQTT power at ");
        logging.Set_log_init(String(powdimmer).c_str());
        logging.Set_log_init("%\r\n");
       
      }
    }
    else if (doc2.containsKey("maxtemp")) { 
      int maxtemp = doc2["maxtemp"]; 
      if (config.maxtemp != maxtemp ) {
        config.maxtemp = maxtemp;
        logging.Set_log_init("MQTT maxtemp at ");  
        logging.Set_log_init(String(maxtemp).c_str());
        logging.Set_log_init("°C\r\n");
        device_dimmer_maxtemp.send(String(maxtemp));
        sysvar.change=1; 
      }
    }
    else if (doc2.containsKey("charge")) { 
      int charge = doc2["charge"]; 
      if (config.charge != charge ) {
        config.charge = charge;
        logging.Set_log_init("MQTT charge at ");
        logging.Set_log_init(String(charge).c_str());
        logging.Set_log_init("W\r\n");
        sysvar.change=1; 
      }
    }
  }
//clear alarm & save
  if (strstr( topic, command_button.c_str() ) != NULL) { 
    if (doc2.containsKey("reset_alarm")) { 
      if (doc2["reset_alarm"] == "1" ) {
        logging.Set_log_init("Clear alarm temp \r\n",true);
        sysvar.security = 0 ;
        device_dimmer_alarm_temp.send(stringBoolMQTT(sysvar.security)); 
        sysvar.change = 1 ;
      }
    }
    else if (doc2.containsKey("save")) { 
      if (doc2["save"] == "1" ) {
        logging.Set_log_init(config.saveConfiguration()); //sauvegarde de la configuration
          
      }
    }


  }


//child mode
  if (strstr( topic, command_select.c_str() ) != NULL) { 
    if (doc2.containsKey("child_mode")) { 
      String childmode = doc2["child_mode"]; 
      if (config.mode != doc2["child_mode"] ) {
        strlcpy(config.mode, doc2["child_mode"], sizeof(config.mode));
        device_dimmer_child_mode.send(String(config.mode));
        logging.Set_log_init("MQTT child mode at ");
        logging.Set_log_init(String(childmode).c_str());
        logging.Set_log_init("\r\n");

      }
    }
  }
  if (strcmp( topic, command_save.c_str() ) == 0) { 
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
        Serial.println("sauvegarde conf mqtt ");
        serializeJson(doc2, buffer);
        Serial.println(config.hostname);
        Serial.println(buffer);
  }


  if (strcmp( topic, HA_status.c_str() ) == 0) { 
        logging.Set_log_init("MQTT HA_status ",true);
        logging.Set_log_init(fixedpayload);
        logging.Set_log_init("\r\n");
        if (strcmp( fixedpayload.c_str(), "online" ) == 0) { 
          logging.Set_log_init("MQTT resend HA discovery \r\n",true);
          HA_discover();
          logging.Set_log_init("MQTT resend all values \r\n",true);
          device_dimmer.send(String(sysvar.puissance));
          device_dimmer_send_power.send(String(sysvar.puissance));
          device_dimmer_power.send(String(sysvar.puissance* config.charge/100));
          if (strcmp(String(config.PVROUTER).c_str() , "http") == 0) { device_dimmer_total_power.send(String(sysvar.puissance_cumul + (sysvar.puissance * config.charge/100)));}
          device_cooler.send(stringBoolMQTT(sysvar.cooler));
          device_temp_master.send(String(sysvar.celsius[sysvar.dallas_maitre]));
          device_dimmer_starting_pow.send(String(config.startingpow));
          device_dimmer_minpow.send(String(config.minpow));
          device_dimmer_maxpow.send(String(config.maxpow));
          device_dimmer_maxtemp.send(String(config.maxtemp));
          if (strcmp(String(config.PVROUTER).c_str() , "http") == 0) {device_dimmer_child_mode.send(String(config.mode));}
          device_dimmer_on_off.send(String(config.dimmer_on_off));

          #ifdef RELAY1
            int relaystate = digitalRead(RELAY1); 
            device_relay1.send(String(relaystate));
          #endif
          #ifdef RELAY2
            relaystate = digitalRead(RELAY2); 
            device_relay2.send(String(relaystate));
          #endif
          if (discovery_temp) {
            for (int i = 0; i < deviceCount; i++) {  //NOSONAR
              device_temp[i].send(String(sysvar.celsius[i]));
            }
            device_temp_master.send(String(sysvar.celsius[sysvar.dallas_maitre]));
            Serial.println(sysvar.celsius[sysvar.dallas_maitre]);
            device_dimmer_alarm_temp.send(stringBoolMQTT(sysvar.security));
            device_dimmer_maxtemp.send(String(config.maxtemp)); 
          }
        }
  }
}

void Mqtt_send_DOMOTICZ(String idx, String value, String name="")
{

  if (config.DOMOTICZ) {
      String nvalue = "0" ; 
      String retour; 
      DynamicJsonDocument doc(128);
      if ( value != "0" ) { nvalue = "2" ; }
      doc["idx"] = idx.toInt();
      doc["nvalue"] = nvalue.toInt();
      doc["svalue"] = value;
      doc["name"] = name;
      serializeJson(doc, retour);
      // si config.Publish est vide, on ne publie pas
      if (strlen(config.Publish) != 0 ) {
        client.publish(config.Publish, retour.c_str(), true);
      }
    }

    if (config.JEEDOM) {
      String jeedom_publish = String(config.Publish) + "/" + idx ; 
      // si config.Publish est vide, on ne publie pas
      if (strlen(config.Publish) != 0 ) {
        client.publish(jeedom_publish.c_str(), value.c_str(), true);
      }
    }

    Serial.println("MQTT SENT");

}


//// communication avec carte fille ( HTTP )

void child_communication(int delest_power, bool equal = false){

    int tmp_puissance_dispo=0 ;
  String baseurl; 
    baseurl = "/?POWER=" + String(delest_power); 
  
  /// Modif RV 20240219
  /// Ajout de " delest_power != 0" pour ne pas envoyer une demande de puissance si on le passe de toutes façons à 0
  if (sysvar.puissance_dispo !=0 && delest_power != 0) {  
    baseurl.concat("&puissance=");
    if ( strcmp(config.child,"") != 0 && strcmp(config.mode,"equal") == 0 ) { tmp_puissance_dispo = sysvar.puissance_dispo/2;}
    else { tmp_puissance_dispo = sysvar.puissance_dispo; }
      baseurl.concat(String(tmp_puissance_dispo)); 
  }
  
  http.begin(domotic_client,config.child,80,baseurl); 
  http.GET();
  http.end(); 

}


//////////// reconnexion MQTT

void connect_and_subscribe() {
  if  (LittleFS.exists("/mqtt.json"))
  {
      if (!client.connected() && WiFi.isConnected()) {
        Serial.print("Attempting MQTT connection...\n");
        connectToMqtt();
        delay(1000); // Attente d'avoir le callback de connexion MQTT avant de faire les subscriptions
      }
      
      
      if (mqttConnected) {
        recreate_topic();
        logging.Set_log_init("Subscribe and publish to MQTT topics\r\n");

        Serial.println("connected");
        logging.Set_log_init("Connected\r\n");

        logging.Set_log_init("Call HA discover\r\n");
        Serial.println("Call HA discover");
        HA_discover();

        logging.Set_log_init("Other subscriptions...\r\n");
        Serial.println("Other subscriptions...");
        if (mqtt_config.mqtt && strlen(config.SubscribePV) !=0 ) {client.subscribe(config.SubscribePV,1);}
        if (mqtt_config.mqtt && strlen(config.SubscribeTEMP) != 0 ) {client.subscribe(config.SubscribeTEMP,1);}
        client.subscribe(command_switch.c_str(),1);
        client.subscribe(command_number.c_str(),1);
        client.subscribe(command_select.c_str(),1);
        client.subscribe(command_button.c_str(),1);

        String node_id = config.say_my_name;
        String save_command = String("Xlyric/sauvegarde/"+ node_id );

        int instant_power = sysvar.puissance;  // 
        Mqtt_send_DOMOTICZ(String(config.IDX), String (sysvar.puissance * config.charge/100));   /// correction 19/04 valeur remonté au dessus du max conf
        device_dimmer.send(String(instant_power)); 
        device_dimmer_power.send(String(instant_power * config.charge/100)); 
      }
  } else {  Serial.println(" Filesystem not present "); delay(5000); }
}
//#define MQTT_HOST IPAddress(192, 168, 1, 20)
char arrayWill[64];// NOSONAR
void async_mqtt_init() {
  String topic_Xlyric = "Xlyric/" + String(config.say_my_name) +"/";
	const String LASTWILL_TOPIC = topic_Xlyric + "status";
	LASTWILL_TOPIC.toCharArray(arrayWill, 64);
  IPAddress ip;
  ip.fromString(config.hostname);
  DEBUG_PRINTLN(ip);
  //client.setClientId(node_id.c_str());
  
  //client.setWill(arrayWill, 2, true, "offline");
 // client.setCredentials(mqtt_config.username, mqtt_config.password);
  //client.onDisconnect(onMqttDisconnect);
  //client.onSubscribe(onMqttSubscribe);
  //client.onMessage(callback);

  //client.setServer(ip, config.port);
 // client.setMaxTopicLength(768); // 1024 -> 768 
 // 

  client.setServer(ip, config.port);
  client.setCallback(callback);
  connectToMqtt();
  
  }

void connectToMqtt() {
  if (!client.connected() ) {
  DEBUG_PRINTLN("Connecting to MQTT...");
    logging.Set_log_init("Connecting to MQTT..." + String(config.say_my_name) + " \r\n");
    delay(500); // pour laisser le temps de se connecter au wifi ou ne pas spam le serveur
    Serial.println(config.hostname);
    IPAddress ip;
    ip.fromString(config.hostname);
    client.setServer(ip, config.port);
    client.setCallback(callback);
    client.setKeepAlive(120);
    client.setBufferSize(1024);
    client.connect(config.say_my_name, mqtt_config.username, mqtt_config.password, arrayWill, 2, true, "offline");
    
  }
  // affig en debug k'état de client.connected
  if (client.connected()) {
    Serial.println("Connected to MQTT.");
    logging.Set_log_init("Connected to MQTT.\r\n");
    /// discovery HA
    recreate_topic();
    HA_discover();
    onMqttConnect(true);
    client.setCallback(callback);
  } 
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  logging.Set_log_init("Connected to MQTT.\r\n");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  String topic_Xlyric = "Xlyric/" + String(config.say_my_name) +"/";
  client.publish(String(topic_Xlyric +"status").c_str(),"online",true);         // Once connected, publish online to the availability topic
  
  if (strcmp(config.PVROUTER, "mqtt") == 0 && strlen(config.SubscribePV) !=0 ) {client.subscribe(config.SubscribePV,1);}
  if (strcmp(config.PVROUTER, "mqtt") == 0 && strlen(config.SubscribeTEMP) != 0 ) {client.subscribe(config.SubscribeTEMP,1);}
  client.subscribe((command_button + "/#").c_str(),1);
  client.subscribe((command_number + "/#").c_str(),1);
  client.subscribe((command_select + "/#").c_str(),1);
  client.subscribe((command_switch + "/#").c_str(),1);
  client.subscribe((HA_status).c_str(),1);
  Serial.println((command_button + "/#").c_str());
  Serial.println((command_number + "/#").c_str());
  Serial.println((command_select + "/#").c_str());
  Serial.println((command_switch + "/#").c_str());
  logging.Set_log_init("MQTT connected \r\n");
  mqttConnected = true;
}


void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  DEBUG_PRINTLN("  qos: ");
  DEBUG_PRINTLN(qos);
}
String stringBoolMQTT(bool mybool){
  String truefalse = "true";
  if (mybool == false ) {truefalse = "false";}
  return String(truefalse);
}
void recreate_topic(){
  String topic_Xlyric = "Xlyric/" + String(config.say_my_name) +"/";
  Serial.println("test "+String(config.say_my_name));
  command_switch = String(topic_Xlyric + "command/switch");
  command_number = String(topic_Xlyric + "command/number");
  command_select = String(topic_Xlyric + "command/select");
  command_button = String(topic_Xlyric + "command/button");
  command_save = String("Xlyric/sauvegarde/"+ node_id );
}

#endif
