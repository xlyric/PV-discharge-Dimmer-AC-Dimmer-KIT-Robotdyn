#include "Arduino.h"
/**************
 *  numeric Dimmer  ( using robodyn dimmer = 
 *  **************
 *  Upgraded By Cyril Poissonnier 2020
 * 
  *    
 * 
 *  ---------------------- OUTPUT & INPUT Pin table ---------------------
 *  +---------------+-------------------------+-------------------------+
 *  |   Board       | INPUT Pin               | OUTPUT Pin              |
 *  |               | Zero-Cross              |                         |
 *  +---------------+-------------------------+-------------------------+
 *  | Lenardo       | D7 (NOT CHANGABLE)      | D0-D6, D8-D13           |
 *  +---------------+-------------------------+-------------------------+
 *  | Mega          | D2 (NOT CHANGABLE)      | D0-D1, D3-D70           |
 *  +---------------+-------------------------+-------------------------+
 *  | Uno           | D2 (NOT CHANGABLE)      | D0-D1, D3-D20           |
 *  +---------------+-------------------------+-------------------------+
 *  | ESP8266       | D1(IO5),    D2(IO4),    | D0(IO16),   D1(IO5),    |
 *  |               | D5(IO14),   D6(IO12),   | D2(IO4),    D5(IO14),   |
 *  |               | D7(IO13),   D8(IO15),   | D6(IO12),   D7(IO13),   |
 *  |               |                         | D8(IO15)                |
 *  +---------------+-------------------------+-------------------------+
 *  | ESP32         | 4(GPI36),   6(GPI34),   | 8(GPO32),   9(GP033),   |
 *  |               | 5(GPI39),   7(GPI35),   | 10(GPIO25), 11(GPIO26), |
 *  |               | 8(GPO32),   9(GP033),   | 12(GPIO27), 13(GPIO14), |
 *  |               | 10(GPI025), 11(GPIO26), | 14(GPIO12), 16(GPIO13), |
 *  |               | 12(GPIO27), 13(GPIO14), | 23(GPIO15), 24(GPIO2),  |
 *  |               | 14(GPIO12), 16(GPIO13), | 25(GPIO0),  26(GPIO4),  |
 *  |               | 21(GPIO7),  23(GPIO15), | 27(GPIO16), 28(GPIO17), |
 *  |               | 24(GPIO2),  25(GPIO0),  | 29(GPIO5),  30(GPIO18), |
 *  |               | 26(GPIO4),  27(GPIO16), | 31(GPIO19), 33(GPIO21), |
 *  |               | 28(GPIO17), 29(GPIO5),  | 34(GPIO3),  35(GPIO1),  |
 *  |               | 30(GPIO18), 31(GPIO19), | 36(GPIO22), 37(GPIO23), |
 *  |               | 33(GPIO21), 35(GPIO1),  |                         |
 *  |               | 36(GPIO22), 37(GPIO23), |                         |
 *  +---------------+-------------------------+-------------------------+
 *  | Arduino M0    | D7 (NOT CHANGABLE)      | D0-D6, D8-D13           |
 *  | Arduino Zero  |                         |                         |
 *  +---------------+-------------------------+-------------------------+
 *  | Arduino Due   | D0-D53                  | D0-D53                  |
 *  +---------------+-------------------------+-------------------------+
 *  | STM32         | PA0-PA15,PB0-PB15       | PA0-PA15,PB0-PB15       |
 *  | Black Pill    | PC13-PC15               | PC13-PC15               |
 *  | BluePill      |                         |                         |
 *  | Etc...        |                         |                         |
 *  +---------------+-------------------------+-------------------------+
 *  
 *  
 *  Work for dimmer on Domoticz or Web command :
 *  http://URL/?POWER=XX 
 *  0 -> 99 
 *  more than 99 = 99  
 *    
 *  Update 2019 04 28 
 *  correct issue full power for many seconds at start 
 *  Update 2020 01 13 
 *  Version 2 with cute web interface
 *  V2.1    with temperature security ( dallas 18b20 ) 
 *          MQTT to Domoticz for temp 
 */




/***************************
 * Librairy
 **************************/

// time librairy   
//#include <NTPClient.h>
// Dimmer librairy 
#include <Arduino.h>
#include <RBDdimmer.h>   /// the corrected librairy  in RBDDimmer-master-corrected.rar , the original has a bug
// Web services
#include <ESP8266WiFi.h>
#include <ESPAsyncWiFiManager.h>    
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266HTTPClient.h> 
// File System
//#include <fs.h>
#include <LittleFS.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson
// ota mise à jour sans fil
#include <AsyncElegantOTA.h>
// Dallas 18b20
#include <OneWire.h>
#include <DallasTemperature.h>
//mqtt
#include <PubSubClient.h>
/// config
#include "config.h"



/***************************
 * Begin Settings
 **************************/

const String VERSION = "Version 2.5" ;

/***************************
 * temperature de sécurité
 **************************/
//#define MAXTEMP 75

// WIFI
// At first launch, create wifi network 'dimmer'  ( pwd : dimmer ) 

//***********************************
//************* Gestion du serveur WEB
//***********************************
// Create AsyncWebServer object on port 80
WiFiClient domotic_client;
// mqtt
void mqtt(String idx, String value);
PubSubClient client(domotic_client);

AsyncWebServer server(80);
DNSServer dns;
HTTPClient http;

//////////

int puissance = 0 ;
int change = 0; 

void reconnect();
void child_communication(int delest_power);

//***********************************
//************* Time
//***********************************
//const long utcOffsetInSeconds = 3600;
//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
int timesync = 0; 
int timesync_refresh = 120; 

// *************************************

//#define USE_SERIAL  SerialUSB //Serial for boards whith USB serial port
#define USE_SERIAL  Serial

//***********************************
//************* Dallas
//***********************************
void dallaspresent ();
float CheckTemperature(String label, byte deviceAddress[12]);
#define TEMPERATURE_PRECISION 10


OneWire  ds(ONE_WIRE_BUS);  //  (a 4.7K resistor is necessary - 5.7K work with 3.3 ans 5V power)
DallasTemperature sensors(&ds);
DeviceAddress insideThermometer;

  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius = 0.00 ;
  byte security = 0;
  int refresh = 30;
  int refreshcount = 0; 

/***************************
 * End Settings
 **************************/

//***********************************
//************* Gestion de la configuration
//***********************************

struct Config {
  char hostname[15];
  int port;
  char Publish[100];
  int IDXTemp;
  int maxtemp;
  int IDXAlarme;
  int IDX;
  int maxpow;
  char child[15];
  char mode[10];
  int minpow;
    
};

const char *filename_conf = "/config.json";
Config config; 

//***********************************
//************* Gestion de la configuration - Lecture du fichier de configuration
//***********************************

// Loads the configuration from a file
void loadConfiguration(const char *filename, Config &config) {
  // Open file for reading
  File configFile = LittleFS.open(filename_conf, "r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<1024> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  
  strlcpy(config.hostname,                  // <- destination
          doc["hostname"] | "192.168.1.20", // <- source
          sizeof(config.hostname));         // <- destination's capacity
  config.port = doc["port"] | 1883;
  strlcpy(config.Publish,                 
          doc["Publish"] | "domoticz/in", 
          sizeof(config.Publish));        
  config.IDXTemp = doc["IDXTemp"] | 200; 
  config.maxtemp = doc["maxtemp"] | 60; 
  config.IDXAlarme = doc["IDXAlarme"] | 202; 
  config.IDX = doc["IDX"] | 201; 
  config.maxpow = doc["maxpow"] | 50; 
  config.minpow = doc["minpow"] | 5;
  strlcpy(config.child,                  
          doc["child"] | "192.168.1.20", 
          sizeof(config.child));         
  strlcpy(config.mode,                  
          doc["mode"] | "off", 
          sizeof(config.mode));
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
  doc["maxpow"] = config.maxpow;
  doc["minpow"] = config.minpow;
  doc["child"] = config.child;
  doc["mode"] = config.mode;
  


  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  configFile.close();
}
 

/***************************
 * init Dimmer
 **************************/

dimmerLamp dimmer(outputPin, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards
//dimmerLamp dimmer(outputPin); //initialase port for dimmer for MEGA, Leonardo, UNO, Arduino M0, Arduino Zero

int outVal = 0;

    //***********************************
    //************* function web 
    //***********************************
const char* PARAM_INPUT_1 = "POWER"; /// paramettre de retour sendmode


String getState() {
  String state; 
  int pow=dimmer.getPower(); 
  state = String(pow) + ";" + String(celsius) ; 
  return String(state);
}


String processor(const String& var){
   Serial.println(var);
  if (var == "STATE"){
    return getState();
  } 
  if (var == "VERSION"){
    return (VERSION);
  } 
  return (VERSION);
} 
 /*
void call_time() {
	//if ( timesync_refresh >= timesync ) {  timeClient.update(); timesync = 0; }
	//else {timesync++;} 
} 
 */
/*
String getTime() {
  String state=""; 
 // state = timeClient.getHours() + ":" + timeClient.getMinutes() ; 
  return String(state);
}
*/

String getconfig() {
  String configweb;  
  configweb = String(config.hostname) + ";" +  config.port+";"+ config.Publish +";"+ config.IDXTemp +";"+ config.maxtemp+ ";"  +  config.IDXAlarme + ";"  + config.IDX + ";"  +  config.maxpow+ ";"  +  config.minpow+ ";" +  config.child+ ";"  +  config.mode ;
  return String(configweb);
}



/// fonction pour mettre en pause ou allumer le dimmer 
void dimmer_on()
{
  if (dimmer.getState()==0) {
    dimmer.setState(ON);
    delay(50);
    }
}

void dimmer_off()
{
  if (dimmer.getState()==1) {
    dimmer.setState(OFF);
    delay(50);
    }
}


    //***********************************
    //************* Setup 
    //***********************************

void setup() {
  Serial.begin(115200);

  /// Correction issue full power at start
  pinMode(outputPin, OUTPUT); 
  digitalWrite(outputPin, LOW);

  //démarrage file system
  LittleFS.begin();
  Serial.println("Demarrage file System");
  
  // configuration dimmer
  dimmer.begin(NORMAL_MODE, ON); //dimmer initialisation: name.begin(MODE, STATE) 
  dimmer.setPower(outVal); 
  
  USE_SERIAL.println("Dimmer Program is starting...");

      //***********************************
    //************* Setup -  récupération du fichier de configuration
    //***********************************
  
  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
  loadConfiguration(filename_conf, config);

  // Create configuration file
  Serial.println(F("Saving configuration..."));
  saveConfiguration(filename_conf, config);

  

    //***********************************
    //************* Setup - Connexion Wifi
    //***********************************
  Serial.print("start Wifiautoconnect");
  AsyncWiFiManager wifiManager(&server,&dns);
  wifiManager.autoConnect("dimmer");
  Serial.print("end Wifiautoconnect");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }


  //Si connexion affichage info dans console
  Serial.println("");
  Serial.print("Connection ok sur le reseau :  ");
 
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 
  Serial.println(ESP.getResetReason());

    //***********************************
    //************* Setup - OTA 
    //***********************************
    AsyncElegantOTA.begin(&server);    // Start ElegantOTA
   
    //***********************************
    //************* Setup - Web pages
    //***********************************
   
   server.on("/",HTTP_ANY, [](AsyncWebServerRequest *request){
    
    if (request->hasParam(PARAM_INPUT_1)) { 
      puissance = request->getParam(PARAM_INPUT_1)->value().toInt();  
      change=1; 
      request->send_P(200, "text/plain", getState().c_str());  
      
      }
    else   request->send(LittleFS, "/index.html", String(), false, processor);

    
  }); 

      server.on("/config.html",HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/config.html", String(), false, processor);
  });

  server.on("/state", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getState().c_str());
  });
/*
  server.on("/time", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getTime().c_str());
  });
*/
  server.on("/all.min.css", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/all.min.css", "text/css");
  });

    server.on("/favicon.ico", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/favicon.ico", "image/png");
  });

  server.on("/fa-solid-900.woff2", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/fa-solid-900.woff2", "text/css");
  });
  
    server.on("/sb-admin-2.js", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/sb-admin-2.js", "text/javascript");
  });

  server.on("/sb-admin-2.min.css", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/sb-admin-2.min.css", "text/css");
  });
  
  
  server.on("/jquery.easing.min.js", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/jquery.easing.min.js", "text/javascript");
  });

  server.on("/config.json", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/config.json", "application/json");
  });

  server.on("/config", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getconfig().c_str());
  });

  server.on("/reset", HTTP_ANY, [](AsyncWebServerRequest *request){
    ESP.restart();
  });
/////////////////////////
////// mise à jour parametre d'envoie vers domoticz et récupération des modifications de configurations
/////////////////////////

server.on("/get", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      ///   /get?paramettre=xxxx
    if (request->hasParam("save")) { Serial.println(F("Saving configuration..."));
                          saveConfiguration(filename_conf, config);   
                            }
                          
   if (request->hasParam("hostname")) { request->getParam("hostname")->value().toCharArray(config.hostname,15);  }
   if (request->hasParam("port")) { config.port = request->getParam("port")->value().toInt();}
   if (request->hasParam("Publish")) { request->getParam("Publish")->value().toCharArray(config.Publish,100);}
   if (request->hasParam("idxtemp")) { config.IDXTemp = request->getParam("idxtemp")->value().toInt();}
   if (request->hasParam("maxtemp")) { config.maxtemp = request->getParam("maxtemp")->value().toInt();}
   if (request->hasParam("IDXAlarme")) { config.IDXAlarme = request->getParam("IDXAlarme")->value().toInt();}
   if (request->hasParam("IDX")) { config.IDX = request->getParam("IDX")->value().toInt();}
   if (request->hasParam("maxpow")) { config.maxpow = request->getParam("maxpow")->value().toInt();}
   if (request->hasParam("minpow")) { config.minpow = request->getParam("minpow")->value().toInt();}
   if (request->hasParam("child")) { request->getParam("child")->value().toCharArray(config.child,15);  }
   if (request->hasParam("mode")) { request->getParam("mode")->value().toCharArray(config.mode,10);  }
   request->send(200, "text/html", getconfig().c_str());

  });


    //***********************************
    //************* Setup -  demarrage du webserver et affichage de l'oled
    //***********************************
  Serial.println("start server");
  server.begin(); 
    
  Serial.println("start 18b20");
  sensors.begin();
    
  /// recherche d'une sonde dallas
  dallaspresent();

  /// MQTT 
  client.connect("Dimmer");
  client.setServer(config.hostname, 1883);
   

}

void loop() {


  //// si la sécurité température est active 
  if ( security == 1 ) { 
      Serial.println("Alert Temp");
      mqtt(String(config.IDXAlarme), String("Alert Temp :" + String(celsius) ));  ///send alert to MQTT
    //// Trigger
      if ( celsius <= config.maxtemp - 5 ) {  
       security = 0 ;
      }
      else {
      dimmer_off();
      }
  }



  /// Changement de la puissance (  pb de Exception 9 si call direct ) 
  if ( change == 1  ) {
    if (puissance >= config.minpow ) {
        dimmer_on();  // if off, switch on 
        if ( puissance > config.maxpow )  dimmer.setPower(config.maxpow); 
        else dimmer.setPower(puissance);
        if ( puissance > config.maxpow && strcmp(config.mode,"delester") == 0 ) { child_communication(puissance-config.maxpow ); } // si mode délest, envoi du surplus
        if (  strcmp(config.mode,"equal") == 0) { child_communication(puissance); }  //si mode equal envoie de la commande vers la carte fille
        
      if ( config.IDX != 0 ) {
      mqtt(String(config.IDX), String(puissance));  // remonté MQTT de la commande
      } 

    }
    else {
        //// si la commande est trop faible on coupe tout partout
        dimmer_off();  
        child_communication(0);
    }
    change = 0; 
  }

 ///// dallas présent >> mesure 
  if ( present == 1 ) { 
    refreshcount ++; 

    sensors.requestTemperatures();
    celsius=CheckTemperature("Inside : ", addr); 

    if ( refreshcount >= refresh ) {
      mqtt(String(config.IDXTemp), String(celsius));  /// remonté MQTT de la température
      refreshcount = 0; 
    } 
  

    delay(500); 
  } 

    //***********************************
    //************* LOOP - Activation de la sécurité
    //***********************************
if ( celsius >= config.maxtemp ) {
  security = 1 ; 
}

///  changement de la puissance




 delay(500); 
}
////fin de loop 


    //***********************************
    //************* récupération d'une température du 18b20
    //***********************************

float CheckTemperature(String label, byte deviceAddress[12]){
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print(label);
  if (tempC == -127.00) {
    Serial.print("Error getting temperature");
  } else {
    Serial.print(" Temp C: ");
    Serial.println(tempC);
    return (tempC); 
   
    
  }  
  return (tempC); 
}


    //***********************************
    //************* Test de la présence d'une 18b20 
    //***********************************

void dallaspresent () {

if ( !ds.search(addr)) {
    Serial.println("Dallas not connected");
    Serial.println();
    ds.reset_search();
    delay(250);
    return ;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

   Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return ;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();    ///  byte 0 > 1 si present
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  present = ");
  Serial.println(present, HEX);


  return ;
   
  }


//////////// reconnexion MQTT

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Dimmer";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


//// envoie de commande MQTT 
void mqtt(String idx, String value)
{
  reconnect();
  String nvalue = "0" ; 
  if ( value != "0" ) { nvalue = "2" ; }
  String message = "  { \"idx\" : " + idx +" ,   \"svalue\" : \"" + value + "\",  \"nvalue\" : " + nvalue + "  } ";


  client.loop();
  client.publish(config.Publish, String(message).c_str(), true);
  
  String jdompub = String(config.Publish) + "/"+idx ;
  client.publish(jdompub.c_str() , value.c_str(), true);

  Serial.println("MQTT SENT");

}


//// communication avec carte fille

void child_communication(int delest_power){

  String baseurl; 
  baseurl = "/?POWER=" + String(delest_power) ; http.begin(domotic_client,config.child,80,baseurl); 
  http.GET();
  http.end(); 

}
