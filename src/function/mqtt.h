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
extern HA device_dimmer_boost;

extern bool boost();
extern bool HA_reconnected;
extern bool discovery_temp;
extern bool alerte;
// extern byte security; // sécurité
extern Logs logging;
extern String devAddrNames[MAX_DALLAS];
extern PubSubClient client;


void connectToMqtt();
void onMqttConnect(bool sessionPresent);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void recreate_topic();
void handleRelay(const JsonDocument& doc, const char* relayKey, int relayPin, const char* relayName, HA& device,bool invert);
void handleNumberParameter(const JsonDocument& doc, const char* key, int& configParam, const char* logMessage, HA& device);

char buffer[1024];// NOSONAR
/// @brief Enregistrement du dimmer sur MQTT pour récuperer les informations remonté par MQTT
/// @param topic
/// @param message
/// @param length

String stringBoolMQTT(bool mybool);

String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
String node_id = String("dimmer-") + node_mac;

String topic_Xlyric = "Xlyric/" + String(config.say_my_name) +"/";

auto command_switch = String(topic_Xlyric + "command/switch");
auto command_number = String(topic_Xlyric + "command/number");
auto command_select = String(topic_Xlyric + "command/select");
auto command_button = String(topic_Xlyric + "command/button");
const String HA_status = String("homeassistant/status");
auto command_save = String("Xlyric/sauvegarde/"+ node_id );

void callback(char* topic, byte* payload, unsigned int length) {

  auto arrivage = new char[length + 1];

  for (unsigned int i=0; i<length; i++) {
    arrivage[i] = (char)payload[i];
  }
  arrivage[length] = '\0';  // Ajouter le caractère nul à la fin

  // debug
  Serial.println("length : " + String(length));
  Serial.println("topic : " + String(topic));
  Serial.println("payload : " + String(arrivage));
  String fixedpayload = ((String)arrivage).substring(0,length);
  JsonDocument doc2;
  deserializeJson(doc2, arrivage);
  /// @brief Enregistrement du dimmer sur MQTT pour récuperer les informations remontées par MQTT
  if (strcmp( topic, config.SubscribePV ) == 0 && doc2["power"].is<int>()) {
    int puissancemqtt = doc2["power"];
    puissancemqtt = puissancemqtt - config.startingpow;
    if (puissancemqtt < 0) puissancemqtt = 0;
    if (sysvar.puissance != puissancemqtt ) {
      sysvar.puissance = puissancemqtt;
      sysvar.change=1;
    } else if (config.dimmer_on_off == 1) {
      device_dimmer.send(String(sysvar.puissance));
      device_dimmer_send_power.send(String(sysvar.puissance));
      device_dimmer_power.send(String(sysvar.puissance*config.charge/100));
    }
  }
  /// @brief Enregistrement temperature
  if (strcmp( topic, config.SubscribeTEMP ) == 0 && doc2["temperature"].is<float>()) {
    Serial.println("lecture temperature MQTT ");
    float temperaturemqtt = doc2["temperature"];
    Serial.println(temperaturemqtt);
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
  /// pour les switch 
  if (strstr( topic, command_switch.c_str() ) != nullptr) {
    logging.Set_log_init("MQTT command switch ",true);
    #ifdef RELAY1
    handleRelay(doc2, "relay1", RELAY1, "relay1", device_relay1,true);
    #endif
    #ifdef RELAY2
    handleRelay(doc2, "relay2", RELAY2, "relay2", device_relay2,false);
    #endif
    if (doc2["on_off"].is<int>()) {
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

  // pour les number
  if (strstr( topic, command_number.c_str() ) != nullptr) {
    handleNumberParameter(doc2, "starting_power", config.startingpow, "MQTT starting_pow at ", device_dimmer_starting_pow);
    handleNumberParameter(doc2, "minpow", config.minpow, "MQTT minpow at ", device_dimmer_minpow);
    handleNumberParameter(doc2, "maxpow", config.maxpow, "MQTT maxpow at ", device_dimmer_maxpow);
    handleNumberParameter(doc2, "maxtemp", config.maxtemp, "MQTT maxtemp at ", device_dimmer_maxtemp);
    handleNumberParameter(doc2, "charge", config.charge, "MQTT charge at ", device_dimmer_power);

    if (doc2["powdimmer"].is<int>()) {
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
  }
  // clear alarm & save
  if (strstr( topic, command_button.c_str() ) != nullptr) {
    Serial.println("MQTT command button "); 
    Serial.print(fixedpayload);
    if (doc2["reset_alarm"].is<int>()) {
        logging.Set_log_init(Clear_alarm_temp,true);
        sysvar.security = false;
        logging.alerte_web="RAS";
        device_dimmer_alarm_temp.send(stringBoolMQTT(sysvar.security));
        sysvar.change = 1;
    }
    else if (doc2["save"].is<int>()) {
        logging.Set_log_init(config.saveConfiguration()); // sauvegarde de la configuration
    }
        // boost
    else if (doc2["boost"].is<int>()) {
        boost();
        device_dimmer_boost.send("1");
    }
  }

  // child mode
  if (strstr( topic, command_select.c_str() ) != nullptr) {
    if (doc2["child_mode"].is<String>()) {
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
    strlcpy(config.hostname, doc2["hostname"], sizeof(config.hostname));
    config.port = doc2["port"];
    strlcpy(config.Publish, doc2["Publish"], sizeof(config.Publish));
    config.IDXTemp = doc2["IDXTemp"];
    config.maxtemp = doc2["maxtemp"];
    config.IDXAlarme = doc2["IDXAlarme"];
    config.IDX = doc2["IDX"];
    config.startingpow = doc2["startingpow"];
    config.minpow = doc2["minpow"];
    config.maxpow = doc2["maxpow"];
    strlcpy(config.child, doc2["child"], sizeof(config.child));
    strlcpy(config.mode, doc2["mode"], sizeof(config.mode));
    strlcpy(config.SubscribePV, doc2["SubscribePV"], sizeof(config.SubscribePV));
    strlcpy(config.SubscribeTEMP, doc2["SubscribeTEMP"], sizeof(config.SubscribeTEMP));
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
      if (
        strcmp(String(config.PVROUTER).c_str(), "http") == 0
        ) {
        device_dimmer_total_power.send(
          String(sysvar.puissance_cumul + (sysvar.puissance * config.charge/100))
          );
      }
      device_cooler.send(stringBoolMQTT(sysvar.cooler));
      device_temp_master.send(String(sysvar.celsius[sysvar.dallas_maitre]));
      device_dimmer_starting_pow.send(String(config.startingpow));
      device_dimmer_minpow.send(String(config.minpow));
      device_dimmer_maxpow.send(String(config.maxpow));
      device_dimmer_maxtemp.send(String(config.maxtemp));
      if (strcmp(String(config.PVROUTER).c_str(), "http") == 0) {
        device_dimmer_child_mode.send(String(config.mode));
      }
      device_dimmer_on_off.send(String(config.dimmer_on_off));
      device_dimmer_boost.send("0");

      #ifdef RELAY1
      int relaystate = !digitalRead(RELAY1); // correction bug de démarrage en GPIO 0
      device_relay1.send(String(relaystate));
      #endif
      #ifdef RELAY2
      relaystate = digitalRead(RELAY2);
      device_relay2.send(String(relaystate));
      #endif
      if (discovery_temp) {
        for (int i = 0; i < deviceCount; i++) {      //NOSONAR
          device_temp[i].send(String(sysvar.celsius[i]));
        }
        device_temp_master.send(String(sysvar.celsius[sysvar.dallas_maitre]));
        Serial.println(sysvar.celsius[sysvar.dallas_maitre]);
        device_dimmer_alarm_temp.send(stringBoolMQTT(sysvar.security));
        device_dimmer_maxtemp.send(String(config.maxtemp));
      }
    }
  }
  
  return;
}

void Mqtt_send_DOMOTICZ(String idx, String value, String name="") {
  if (config.DOMOTICZ) {
    String nvalue = "0";
    String retour;
    JsonDocument doc;
    if ( value != "0" ) { nvalue = "2"; }
    doc["idx"] = idx.toInt();
    doc["nvalue"] = nvalue.toInt();
    doc["svalue"] = value;
    doc["name"] = name;
    serializeJson(doc, retour);
    // si config.Publish est vide, on ne publie pas
    if (strlen(config.Publish) != 0 ) { // NOSONAR
      sysvar.wait_unlock_mqtt();
      client.publish(config.Publish, retour.c_str(), true);
    }
  }

  if (config.JEEDOM) {
    auto jeedom_publish = String(config.Publish) + "/" + idx;
    // si config.Publish est vide, on ne publie pas
    if (strlen(config.Publish) != 0 ) { // NOSONAR
      sysvar.wait_unlock_mqtt();
      client.publish(jeedom_publish.c_str(), value.c_str(), true);
    }
  }
  Serial.println("MQTT SENT");
}


//// communication avec carte fille ( HTTP )
void child_communication(int delest_power, bool equal = false){
  int tmp_puissance_dispo=0;
  String baseurl;
  baseurl = "/?POWER=" + String(delest_power);

  /// Modif RV 20240219
  /// Ajout de " delest_power != 0" pour ne pas envoyer une demande de puissance si on le passe de toutes façons à 0
  if (sysvar.puissance_dispo !=0 && delest_power != 0) {
    baseurl.concat("&puissance=");
    if (
      strcmp(config.child, "") != 0
      && strcmp(config.mode,"equal") == 0
      ) {
      tmp_puissance_dispo = sysvar.puissance_dispo/2;
    } else {
      tmp_puissance_dispo = sysvar.puissance_dispo;
    }
    baseurl.concat(String(tmp_puissance_dispo));
  }

  http.begin(domotic_client,config.child,80,baseurl);
  http.GET();
  http.end();
}


//////////// reconnexion MQTT

void connect_and_subscribe() {
    if (!client.connected() && WiFi.isConnected()) {
      Serial.print("Attempting MQTT connection...\n");
      connectToMqtt();
      delay(1000);   // Attente d'avoir le callback de connexion MQTT avant de faire les subscriptions
    }

    if (mqttConnected) {
      recreate_topic();
      logging.Set_log_init(Subscribe_MQTT);

      Serial.println("connected");
      logging.Set_log_init("Connected\r\n");

      logging.Set_log_init("Call HA discover\r\n");
      Serial.println("Call HA discover");
      HA_discover();

      logging.Set_log_init("Other subscriptions...\r\n");
      Serial.println("Other subscriptions...");
      if (mqtt_config.mqtt && strlen(config.SubscribePV) !=0 ) {client.subscribe(config.SubscribePV,1);} // NOSONAR
      if (mqtt_config.mqtt && strlen(config.SubscribeTEMP) != 0 ) {client.subscribe(config.SubscribeTEMP,1);} // NOSONAR
      client.subscribe(command_switch.c_str(),1);
      client.subscribe(command_number.c_str(),1);
      client.subscribe(command_select.c_str(),1);
      client.subscribe(command_button.c_str(),1);
      Serial.print("Subscribe to ");
      Serial.println(command_switch.c_str());
      

      String node_id = config.say_my_name;
      auto save_command = String("Xlyric/sauvegarde/"+ node_id );

      int instant_power = sysvar.puissance;
      /// correction 19/04 valeur remonté au dessus du max conf
      Mqtt_send_DOMOTICZ(String(config.IDX), String (sysvar.puissance * config.charge/100));
      device_dimmer.send(String(instant_power));
      device_dimmer_power.send(String(instant_power * config.charge/100));
    }

}
// #define MQTT_HOST IPAddress(192, 168, 1, 20)
char arrayWill[64];// NOSONAR
void async_mqtt_init() {
  String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  String topic_Xlyric_1 = "Xlyric/dimmer-" + node_mac +"/";;
  // String topic_Xlyric = "Xlyric/" + String(config.say_my_name) +"/";
  const String LASTWILL_TOPIC = topic_Xlyric_1 + "status";
  LASTWILL_TOPIC.toCharArray(arrayWill, 64);
  IPAddress ip;
  ip.fromString(config.hostname);
  DEBUG_PRINTLN(ip);
  client.setServer(ip, config.port);
  client.setCallback(callback);
  connectToMqtt();

}

void connectToMqtt() {
  if (!client.connected() ) {
    DEBUG_PRINTLN(Connecting_MQTT);
    logging.Set_log_init(String(Connecting_MQTT) + String(config.say_my_name) + " \r\n");
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
  String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  String topic_Xlyric = "Xlyric/dimmer-" + node_mac +"/";;

  // Once connected, publish online to the availability topic
  sysvar.wait_unlock_mqtt();
  client.publish(String(topic_Xlyric +"status").c_str(),"online",true);

  if (strlen(config.SubscribePV) !=0 ) {
    client.subscribe(config.SubscribePV,1);
    Serial.println(config.SubscribePV);
  }
  if (strlen(config.SubscribeTEMP) != 0 ) {
    client.subscribe(config.SubscribeTEMP,1);
    Serial.println(config.SubscribeTEMP);
  }
  client.subscribe((command_button + "/#").c_str(),1);
  client.subscribe((command_number + "/#").c_str(),1);
  client.subscribe((command_select + "/#").c_str(),1);
  client.subscribe((command_switch + "/#").c_str(),1);
  client.subscribe((HA_status).c_str(),1);
  Serial.println((command_button + "/#").c_str());
  Serial.println((command_number + "/#").c_str());
  Serial.println((command_select + "/#").c_str());
  Serial.println((command_switch + "/#").c_str());
  logging.Set_log_init(MQTT_connected);
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
  Serial.println(command_button);

  command_save = String("Xlyric/sauvegarde/"+ node_id );
}

/// @brief  Gestion des relais ( factorisation )
/// @param doc 
/// @param relayKey 
/// @param relayPin 
/// @param relayName 
/// @param device 
void handleRelay(const JsonDocument& doc, const char* relayKey, int relayPin, const char* relayName, HA& device,bool invert=false) {
    if (doc[relayKey].is<int>()) {
        int relay = doc[relayKey];
        if (invert) {
            digitalWrite(relayPin, relay == 0 ? HIGH : LOW);
        }
        else {
            digitalWrite(relayPin, relay == 0 ? LOW : HIGH);
        }

        logging.Set_log_init(String(relayName) + " at ");
        logging.Set_log_init(String(relay).c_str());
        logging.Set_log_init("\r\n");
        device.send(String(relay));
    }
}

/// @brief  Gestion des paramètres numériques ( factorisation )
void handleNumberParameter(const JsonDocument& doc, const char* key, int& configParam, const char* logMessage, HA& device) {
    if (doc[key].is<int>()) {
        int value = doc[key];
        if (configParam != value) {
            configParam = value;
            logging.Set_log_init(logMessage);
            logging.Set_log_init(String(value).c_str());
            logging.Set_log_init("%\r\n");
            device.send(String(value));
            sysvar.change = 1;
        }
    }
}

#endif
