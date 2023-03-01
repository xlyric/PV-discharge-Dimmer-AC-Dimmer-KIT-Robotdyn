#ifndef WEB_FUNCTIONS
#define WEB_FUNCTIONS

// #include <ESP8266WiFi.h>
#include <ESPAsyncWiFiManager.h>    
// #include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
// #include <ESP8266HTTPClient.h> 
#include "config/config.h"
#include "function/littlefs.h"
#include "function/ha.h"

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

extern Mqtt mqtt_config; 
extern Config config; 
extern System sysvar;

extern dimmerLamp dimmer; 
extern DNSServer dns;

AsyncWebServer server(80);

AsyncWiFiManager wifiManager(&server,&dns);

extern bool AP; 

extern HA device_dimmer; 
extern HA device_temp; 
extern HA device_relay1;
extern HA device_relay2;
// extern HA device_cooler;
extern HA device_dimmer_on_off;
extern HA device_dimmer_child_mode;
extern HA device_dimmer_maxpow;
extern HA device_dimmer_minpow;
extern HA device_dimmer_starting_pow;
extern HA device_dimmer_maxtemp;
extern String dimmername;





const char* PARAM_INPUT_1 = "POWER"; /// paramettre de retour sendmode
const char* PARAM_INPUT_2 = "OFFSET"; /// paramettre de retour sendmode

extern char buffer[1024];

String getmqtt(); 
String getconfig(); 
String getState();
String textnofiles();
String processor(const String& var);
String getServermode(String Servermode);
String stringbool(bool mybool);
String switchstate(int state);
String readmqttsave();
extern String getlogs(); 



void call_pages() {

  server.on("/",HTTP_ANY, [](AsyncWebServerRequest *request){
    
    if  (LittleFS.exists("/index.html")) {
      if (request->hasParam(PARAM_INPUT_1)) { 

        sysvar.puissance = request->getParam(PARAM_INPUT_1)->value().toInt(); 
        logs += "HTTP power at " + String(sysvar.puissance) + "\r\n"; 
        sysvar.change=1; 
        request->send_P(200, "text/plain", getState().c_str());  
      }
      else if (request->hasParam(PARAM_INPUT_2)) { 
        config.startingpow = request->getParam(PARAM_INPUT_2)->value().toInt(); 
        logs += "HTTP power at " + String(config.startingpow) + "\r\n"; 

        sysvar.change=1; 
        request->send_P(200, "text/plain", getState().c_str());  
      }
      else if (request->hasParam(PARAM_INPUT_2)) { 
        config.startingpow = request->getParam(PARAM_INPUT_2)->value().toInt(); 
        logs += "HTTP power at " + String(config.startingpow) + "\r\n"; 
        sysvar.change=1; 
        request->send_P(200, "text/plain", getState().c_str());
      }
      else  { 
            if (!AP) {
              request->send(LittleFS, "/index.html", String(), false, processor);

            }
            else {
              request->send(LittleFS, "/index-AP.html", String(), false, processor);
            }
          }
    }
    else
    { 
      request->send_P(200, "text/html", textnofiles().c_str());
    }
    
  }); 

  server.on("/config.html",HTTP_ANY, [](AsyncWebServerRequest *request){
    if  (LittleFS.exists("/config.html")) {
        if (!AP) {
            request->send(LittleFS, "/config.html", String(), false, processor);

            //request->send(LittleFS, "/config.html", "text/html");
        }
        else {
            request->send(LittleFS, "/config-AP.html", String(), false, processor);
        }

    }
    else
    { 
      request->send_P(200, "text/plain", textnofiles().c_str());
    }
  });

  server.on("/state", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getState().c_str());
  });

  server.on("/resetwifi", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", "Resetting Wifi and reboot");
    wifiManager.resetSettings();
  });

  server.on("/reboot", HTTP_ANY, [](AsyncWebServerRequest *request){
   // request->send_P(200, "text/plain","Restarting");
    request->redirect("/");
    config.restart = true;
  });

//// compressé
  server.on("/all.min.css", HTTP_ANY, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/css/all.min.css.gz", "text/css");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });


    server.on("/favicon.ico", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/favicon.ico", "image/png");
  });

///déja compressé
  server.on("/fa-solid-900.woff2", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/css/fa-solid-900.woff2", "text/css");
  });
  
    server.on("/sb-admin-2.js", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/js/sb-admin-2.js", "text/javascript");
  });

//// compressé
  server.on("/sb-admin-2.min.css", HTTP_ANY, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/sb-admin-2.min.css.gz", "text/css");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  
//// compressé
  server.on("/jquery.easing.min.js", HTTP_ANY, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/js/jquery.easing.min.js.gz", "text/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/config.json", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/config.json", "application/json");
  });
  
  server.on("/mqtt.json", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/mqtt.json", "application/json");
  });

//// compressé
  server.on("/mqtt.html", HTTP_ANY, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/mqtt.html.gz", "text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
   // request->send(LittleFS, "/mqtt.html", "text/html");
  });

//// compressé
  server.on("/log.html", HTTP_ANY, [](AsyncWebServerRequest *request){
    //request->send(LittleFS, "/log.html", "text/html");
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/log.html.gz", "text/html");
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
  });

  server.on("/getmqtt", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain",  getmqtt().c_str()); 
  });

  server.on("/config", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getconfig().c_str());
  });

  server.on("/reset", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain","Restarting");
    ESP.restart();
  });

  server.on("/cs", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getlogs().c_str());
    logs="197}11}1";
  });



  server.on("/save", HTTP_ANY, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/config.json", String(), true);
  });

  server.on("/readmqtt", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", readmqttsave().c_str());
    });


/////////////////////////
////// mise à jour parametre d'envoie vers domoticz et récupération des modifications de configurations
/////////////////////////

server.on("/get", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      ///   /get?paramettre=xxxx
    if (request->hasParam("save")) { Serial.println(F("Saving configuration..."));
                          saveConfiguration(filename_conf, config);   
                            }
                          
   if (request->hasParam("hostname")) { request->getParam("hostname")->value().toCharArray(config.hostname,16);  }
   if (request->hasParam("port")) { config.port = request->getParam("port")->value().toInt();}
   if (request->hasParam("Publish")) { request->getParam("Publish")->value().toCharArray(config.Publish,100);}
   if (request->hasParam("idxtemp")) { config.IDXTemp = request->getParam("idxtemp")->value().toInt();}
   if (request->hasParam("maxtemp")) { 
    config.maxtemp = request->getParam("maxtemp")->value().toInt();
    if (!AP && mqtt_config.mqtt) { device_dimmer_maxtemp.send(String(config.maxtemp));}
   }
   if (request->hasParam("IDXAlarme")) { config.IDXAlarme = request->getParam("IDXAlarme")->value().toInt();}
   if (request->hasParam("IDX")) { config.IDX = request->getParam("IDX")->value().toInt();}
   if (request->hasParam("startingpow")) { config.startingpow = request->getParam("startingpow")->value().toInt();
    if (!AP && mqtt_config.mqtt) { device_dimmer_starting_pow.send(String(config.startingpow));}
   }
   if (request->hasParam("minpow")) { 
    config.minpow = request->getParam("minpow")->value().toInt();
    if (!AP && mqtt_config.mqtt) { device_dimmer_minpow.send(String(config.minpow));}
   }
   if (request->hasParam("maxpow")) { 
    config.maxpow = request->getParam("maxpow")->value().toInt();
    if (!AP && mqtt_config.mqtt) { device_dimmer_maxpow.send(String(config.maxpow));}
   }
   if (request->hasParam("child")) { request->getParam("child")->value().toCharArray(config.child,15);  }
   if (request->hasParam("mode")) { 
    request->getParam("mode")->value().toCharArray(config.mode,10);  
    if (!AP && mqtt_config.mqtt) { device_dimmer_child_mode.send(String(config.mode));}
   }

   if (request->hasParam("SubscribePV")) { request->getParam("SubscribePV")->value().toCharArray(config.SubscribePV,100);}
   if (request->hasParam("SubscribeTEMP")) { request->getParam("SubscribeTEMP")->value().toCharArray(config.SubscribeTEMP,100);}
   if (request->hasParam("dimmer_on_off")) { 
    config.dimmer_on_off = request->getParam("dimmer_on_off")->value().toInt();
    if (!AP && mqtt_config.mqtt) { device_dimmer_on_off.send(String(config.dimmer_on_off));}
   }
   if (request->hasParam("mqttuser")) { request->getParam("mqttuser")->value().toCharArray(mqtt_config.username,50);  }
   if (request->hasParam("mqttpassword")) { request->getParam("mqttpassword")->value().toCharArray(mqtt_config.password,50);
   savemqtt(mqtt_conf, mqtt_config); 
   saveConfiguration(filename_conf, config);
   }
  //Ajout des relais
  // #ifdef STANDALONE
  #ifdef RELAY1
   if (request->hasParam("relay1")) { int relay = request->getParam("relay1")->value().toInt(); 

        if ( relay == 0) { digitalWrite(RELAY1 , LOW); }
        else if (relay == 1) { digitalWrite(RELAY1 , HIGH); } 
        else if (relay == 2) { digitalWrite(RELAY1, !digitalRead(RELAY1)); }
        int relaystate = digitalRead(RELAY1); 
        char str[8];
        itoa( relaystate, str, 10 );
        request->send(200, "text/html", str );
        if (!AP && mqtt_config.mqtt) { device_relay1.send(String(relaystate));}
    }
  #endif
  #ifdef RELAY2
    if (request->hasParam("relay2")) { int relay = request->getParam("relay2")->value().toInt(); 

        if ( relay == 0) { digitalWrite(RELAY2 , LOW); }
        else if (relay == 1) { digitalWrite(RELAY2 , HIGH); } 
        else if (relay == 2) { digitalWrite(RELAY2, !digitalRead(RELAY2)); }
        int relaystate = digitalRead(RELAY2); 
        char str[8];
        itoa( relaystate, str, 10 );
        request->send(200, "text/html", str );
        if (!AP && mqtt_config.mqtt) { device_relay2.send(String(relaystate));}
    }
  #endif 

   //// for check boxs in web pages  
   if (request->hasParam("servermode")) {String inputMessage = request->getParam("servermode")->value();
                                            getServermode(inputMessage);
                                            request->send(200, "text/html", getconfig().c_str());
                                            saveConfiguration(filename_conf, config);
                                            savemqtt(mqtt_conf, mqtt_config); 
                                        }

   request->send(200, "text/html", getconfig().c_str());
  });
  


}


/// @brief Pages de traitement 
/// @return 
String getState() {
  String state; 
  int pow=dimmer.getPower(); 
  
  String routeur="PV-ROUTER";

  state = String(pow) + ";" + String(sysvar.celsius) + ";" + String(outputPin) + ";" + String(zerocross)+ ";" + String(WiFi.SSID().substring(0,9)) + ";" + routeur.compareTo(WiFi.SSID().substring(0,9)); 
  return String(state);
}

String textnofiles() {
  String state = "<html><body>Filesystem is not present. <a href='https://ota.apper-solaire.org/firmware/littlefs-dimmer.bin'>download it here</a> <br>and after  <a href='/update'>upload on the ESP here </a></body></html>" ; 
  return String(state);
}

String processor(const String& var){
  // Serial.println(var);
  if (var == "STATE"){
    return getState();
  } 
  if (var == "VERSION"){
    return (VERSION);
  } 
  if (var == "NAME"){
    return (dimmername);
  } 
  return ("N/A");
} 


String getconfig() {
  String configweb;  

  configweb = String(config.hostname) + ";" +  config.port+";"+ config.Publish +";"+ config.IDXTemp +";"+ config.maxtemp+ ";"  +  config.IDXAlarme + ";"  + config.IDX + ";"  +  config.startingpow+ ";"  +  config.minpow+ ";" +  config.maxpow+ ";"  +  config.child+ ";"  +  config.mode + ";" + config.SubscribePV + ";" + config.SubscribeTEMP + ";" + config.dimmer_on_off ;

  return String(configweb);
}

String getmqtt() {

    String retour =String(config.hostname) + ";" + String(config.Publish) + ";" + String(mqtt_config.username) + ";" + String(mqtt_config.password) + ";" + stringbool(mqtt_config.mqtt)+ ";" + String(config.port) ;
    return String(retour) ;
  }

String readmqttsave(){
        String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
        String node_id = String("dimmer-") + node_mac; 
        String save_command = String("sauvegarde/"+ node_id );
        client.subscribe(save_command.c_str());
        return String("<html><head><meta http-equiv='refresh' content='5;url=config.html' /></head><body><h1>config restauree, retour au setup dans 5 secondes, pensez a sauvegarder sur la flash </h1></body></html>");
}

String getServermode(String Servermode) {
  if ( Servermode == "MQTT" ) {   mqtt_config.mqtt = !mqtt_config.mqtt; }
return String(Servermode);
}

String stringbool(bool mybool){
  String truefalse = "true";
  if (mybool == false ) {truefalse = "false";}
  return String(truefalse);
  }

// String switchstate(int state){
//   String statestring ="OFF";
//   if (state==1) statestring ="ON";
//   return (statestring);
// }

#endif