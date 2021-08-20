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
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// Dallas 18b20
#include <OneWire.h>
#include <DallasTemperature.h>
//mqtt
#include <PubSubClient.h>



/***************************
 * Begin Settings
 **************************/

const String VERSION = "Version 2.2" ;

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
#define outputPin  D0 
#define zerocross  D1 // for boards with CHANGEBLE input pins


//***********************************
//************* Dallas
//***********************************
void dallaspresent ();
float CheckTemperature(String label, byte deviceAddress[12]);
#define TEMPERATURE_PRECISION 10
#define ONE_WIRE_BUS D2

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
  int IDX;
  int IDXAlarme;
  int maxtemp;
  char Publish[100];
  
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
  config.port = doc["port"] | 1883;
  strlcpy(config.hostname,                  // <- destination
          doc["hostname"] | "192.168.1.20", // <- source
          sizeof(config.hostname));         // <- destination's capacity
  config.IDX = doc["IDX"] | 61; 
  config.IDXAlarme = doc["IDXAlarme"] | 100; 
  config.maxtemp = doc["maxtemp"] | 75; 
  strlcpy(config.Publish,                  // <- destination
          doc["Publish"] | "domoticz/in", // <- source
          sizeof(config.Publish));         // <- destination's capacity
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
  StaticJsonDocument<512> doc;

  // Set the values in the document
  doc["hostname"] = config.hostname;
  doc["port"] = config.port;
  doc["IDX"] = config.IDX;
  doc["IDXAlarme"] = config.IDXAlarme;
  doc["maxtemp"] = config.maxtemp;
  doc["Publish"] = config.Publish;
  


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
 
void call_time() {
	//if ( timesync_refresh >= timesync ) {  timeClient.update(); timesync = 0; }
	//else {timesync++;} 
} 
 
String getTime() {
  String state=""; 
 // state = timeClient.getHours() + ":" + timeClient.getMinutes() ; 
  return String(state);
}

String getconfig() {
  String configweb;  
  configweb = String(config.hostname) + ";" +  config.port + ";"  + config.IDX + ";"  +  config.IDXAlarme +";"+config.maxtemp+";"+config.Publish;
  return String(configweb);
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

   ArduinoOTA.setHostname("PV Dimmer"); 
   ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

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

  server.on("/state", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getState().c_str());
  });

  server.on("/time", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getTime().c_str());
  });

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

  server.on("/config.json", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/config.json", "application/json");
  });

/////////////////////////
////// mise à jour parametre d'envoie vers domoticz et récupération des modifications de configurations
/////////////////////////

server.on("/set", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      ///   /set?paramettre=xxxx
    if (request->hasParam("save")) { Serial.println(F("Saving configuration..."));
                          saveConfiguration(filename_conf, config);   
                            }
                          
   if (request->hasParam("hostname")) { request->getParam("hostname")->value().toCharArray(config.hostname,15);  }
   if (request->hasParam("port")) { config.port = request->getParam("port")->value().toInt();}
   if (request->hasParam("IDX")) { config.IDX = request->getParam("IDX")->value().toInt();}
   if (request->hasParam("IDXAlarme")) { config.IDXAlarme = request->getParam("IDXAlarme")->value().toInt();}
   if (request->hasParam("maxtemp")) { config.maxtemp = request->getParam("maxtemp")->value().toInt();}
   if (request->hasParam("Publish")) { request->getParam("Publish")->value().toCharArray(config.Publish,100);}
   request->send(200, "text/html", getconfig().c_str());

  });






    //***********************************
    //************* Setup -  demarrage du webserver et affichage de l'oled
    //***********************************
  Serial.println("start server");
  server.begin(); 
  
 // Serial.println("start ntp");
 // timeClient.begin();
 // timeClient.update();
  
  Serial.println("start 18b20");
  sensors.begin();
  /*if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  sensors.setResolution(insideThermometer, 9);
*/
  
  dallaspresent();

  client.connect("Dimmer");
  client.setServer(config.hostname, 1883);
  

}

void loop() {

  if ( security == 1 ) { 
      Serial.println("Alerte Temp");
      mqtt(String(config.IDXAlarme), String("Alerte Temp :" + String(celsius) ));  
    //// Trigger
      if ( celsius <= config.maxtemp - 5 ) { 
       security = 0 ;
      }
      else {
      dimmer.setPower(0); 
      }
  }



  /// Changement de la puissance (  pb de Exception 9 si call direct ) 
  if ( change == 1  ) {

  dimmer.setPower(puissance); 
  change = 0; 
  }

 ///// dallas présent >> mesure 
  if ( present == 1 ) { 
 refreshcount ++; 

  sensors.requestTemperatures();
  celsius=CheckTemperature("Inside : ", addr); 

  if ( refreshcount >= refresh ) {
    mqtt(String(config.IDX), String(celsius));  
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



 ArduinoOTA.handle();
 delay(500); 
}



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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
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



void mqtt(String idx, String value)
{
  String nvalue = "0" ; 
  if ( value != "0" ) { nvalue = "2" ; }
String message = "  { \"idx\" : " + idx +" ,   \"svalue\" : \"" + value + "\",  \"nvalue\" : " + nvalue + "  } ";

//  if (!client.connected()) {
//    reconnect();
 // }
client.loop();
  client.publish(config.Publish, String(message).c_str(), true);
  
}



