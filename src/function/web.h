#ifndef WEB_FUNCTIONS
#define WEB_FUNCTIONS

// #include <ESP8266WiFi.h>
#include <ESPAsyncWiFiManager.h>  
#include <stdlib.h>

// #include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
// #include <ESP8266HTTPClient.h> 
#include "config/config.h"
#include "function/littlefs.h"
#include "function/ha.h"
#include "function/minuteur.h"



#if defined(ESP32) || defined(ESP32ETH)
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
extern Programme programme; 
extern Programme programme_relay1; 
extern Programme programme_relay2; 

extern dimmerLamp dimmer; 
extern DNSServer dns;
extern byte security; 

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

//extern char buffer[1024];

String getmqtt(); 
String getconfig(); 
String getState();
String textnofiles();
String processor(const String& var);
String getServermode(String Servermode);

String switchstate(int state);
String readmqttsave();
String getMinuteur(const Programme& minuteur);
extern String getlogs(); 



void call_pages() {

/// page de index et récupération des requetes de puissance
  server.on("/",HTTP_ANY, [](AsyncWebServerRequest *request){
    
    if  (LittleFS.exists("/index.html")) {
      DEBUG_PRINTLN("------------------");
      DEBUG_PRINTLN(sysvar.puissance);
      /// si requete sur POWER

#ifdef DEBUG
      Serial.println("index.html");
  int paramsCount = request->params();
  for (int i = 0; i < paramsCount; i++) {
    AsyncWebParameter *param = request->getParam(i);
    if (param->isPost()) {
      Serial.printf("Paramètre POST : %s = %s\n", param->name().c_str(), param->value().c_str());
    } else {
      Serial.printf("Paramètre GET : %s = %s\n", param->name().c_str(), param->value().c_str());
    }
  }
#endif

      if (request->hasParam(PARAM_INPUT_1)) { 
        int input=request->getParam(PARAM_INPUT_1)->value().toInt();
        
        /// si remonté de puissance dispo alors prioritaire
         if (request->hasParam("puissance")) { 
            /// on recupere la puissance disponible  
            
            int dispo = request->getParam("puissance")->value().toInt();
            DEBUG_PRINTLN("puissance="+String(dispo));
            /// on la converti en pourcentage de charge et config.dispo contient la puissance disponible en W 
            config.dispo = dispo;
            sysvar.puissance_dispo = dispo;
            dispo= (100*dispo/config.charge); 
            /// si POWER=0 on coupe tout
            if (input==0) {sysvar.puissance = 0 ;dispo=0; } 
            ///   si au max, on prend la puissance dispo 
            //else if  (dimmer.getPower() == config.maxpow ) { config.dispo = request->getParam("puissance")->value().toInt(); }
            /// si on dépasse le max, on prend la puissance dispo restante 
            else if  (dimmer.getPower() + dispo >= config.maxpow ) { 
              config.dispo = (config.dispo - ((config.maxpow - dimmer.getPower()) * config.charge / 100));  
              dispo = (100*config.dispo/config.charge); // on recalcule le pourcentage
              logs.concat("puissance max \r\n");
            }
            //else if  (sysvar.puissance + dispo > config.maxpow ) { config.dispo = (dispo - (config.maxpow - sysvar.puissance)) * config.charge / 100;  }
            // on égalise
            if ( strcmp(config.mode,"equal") == 0) {
              sysvar.puissance = sysvar.puissance + dispo/2;
            }
            else {
              sysvar.puissance = sysvar.puissance + dispo; 
            }
         }
         else
         { sysvar.puissance = input;  config.dispo = 0; DEBUG_PRINTLN("input="+String(input));}
        
        // si config.child = 0.0.0.0 alors max = 100 
        int max = 200;
        if (strcmp(config.child,"") == 0 || strcmp(config.mode,"off") ==0 ) { max = 100; } 
        if (sysvar.puissance >= max) {sysvar.puissance = max; }
        logs.concat("HTTP power at " + String(sysvar.puissance)+"\r\n");
        sysvar.change=1; 
        String pb=getState().c_str(); 
        pb = pb +String(sysvar.puissance) +" " + String(input) +" " + String(sysvar.puissance_dispo) ;
        request->send_P(200, "text/plain", pb.c_str() );  
      } 
      else if (request->hasParam(PARAM_INPUT_2)) { 
        config.startingpow = request->getParam(PARAM_INPUT_2)->value().toInt(); 
        logs.concat("HTTP power at " + String(config.startingpow)+"\r\n");

        sysvar.change=1; 
        request->send_P(200, "text/plain", getState().c_str());  
      }
      else if (request->hasParam(PARAM_INPUT_2)) { 
        config.startingpow = request->getParam(PARAM_INPUT_2)->value().toInt(); 
        logs.concat("HTTP power at " + String(config.startingpow)+"\r\n");
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
    
    DEBUG_PRINTLN(sysvar.puissance);
    DEBUG_PRINTLN("------------------");
  }); 

/// page de config et récupération des requetes de config
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
    config.restart = true;
  });

  server.on("/reboot", HTTP_ANY, [](AsyncWebServerRequest *request){
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
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/css/sb-admin-2.min.css.gz", "text/css");
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

  server.on("/wifi.json", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/wifi.json", "application/json");
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


/////////// minuteur 
  server.on("/minuteur.html", HTTP_ANY, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/minuteur.html", "text/html");
    //response->addHeader("Content-Encoding", "gzip");
    request->send(response);
   // request->send(LittleFS, "/mqtt.html", "text/html");
  });

    server.on("/minuteur1.html", HTTP_ANY, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/minuteurR1.html", "text/html");
    //response->addHeader("Content-Encoding", "gzip");
    request->send(response);
   // request->send(LittleFS, "/mqtt.html", "text/html");
  });

    server.on("/minuteur2.html", HTTP_ANY, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/minuteurR2.html", "text/html");
    //response->addHeader("Content-Encoding", "gzip");
    request->send(response);
   // request->send(LittleFS, "/mqtt.html", "text/html");
  });

  server.on("/getminiteur", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("dimmer")) { request->send(200, "application/json",  getMinuteur(programme));  }
    if (request->hasParam("relay1")) { request->send(200, "application/json",  getMinuteur(programme_relay1)); }
    if (request->hasParam("relay2")) { request->send(200, "application/json",  getMinuteur(programme_relay2)); }
    
    //request->send(200, "application/json",  getminuteur(programme_relay2).c_str()); 
  });

  server.on("/setminiteur", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    String name; 
    if (request->hasParam("dimmer")) { 
            if (request->hasParam("heure_demarrage")) { request->getParam("heure_demarrage")->value().toCharArray(programme.heure_demarrage,6);  }
            if (request->hasParam("heure_arret")) { request->getParam("heure_arret")->value().toCharArray(programme.heure_arret,6);  }
            if (request->hasParam("temperature")) { programme.temperature = request->getParam("temperature")->value().toInt();  programme.saveProgramme(); }
       request->send(200, "application/json",  getMinuteur(programme));  
    }
    if (request->hasParam("relay1")) { 
            if (request->hasParam("heure_demarrage")) { request->getParam("heure_demarrage")->value().toCharArray(programme_relay1.heure_demarrage,6);  }
            if (request->hasParam("heure_arret")) { request->getParam("heure_arret")->value().toCharArray(programme_relay1.heure_arret,6);  }
            if (request->hasParam("temperature")) { programme_relay1.temperature = request->getParam("temperature")->value().toInt();  programme_relay1.saveProgramme(); }
      request->send(200, "application/json",  getMinuteur(programme_relay1)); 
    }
    if (request->hasParam("relay2")) { 
            if (request->hasParam("heure_demarrage")) { request->getParam("heure_demarrage")->value().toCharArray(programme_relay2.heure_demarrage,6);  }
            if (request->hasParam("heure_arret")) { request->getParam("heure_arret")->value().toCharArray(programme_relay2.heure_arret,6);  }
            if (request->hasParam("temperature")) { programme_relay2.temperature = request->getParam("temperature")->value().toInt();  programme_relay2.saveProgramme(); }
      request->send(200, "application/json",  getMinuteur(programme_relay2)); 
    }
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
   if (request->hasParam("charge")) { config.charge = request->getParam("charge")->value().toInt();}
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
    if (request->hasParam("charge")) { 
    config.charge = request->getParam("charge")->value().toInt();
    if (!AP && mqtt_config.mqtt) { device_dimmer_charge.send(String(config.charge));}
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

//// minuteur 
   if (request->hasParam("heure_demarrage")) { request->getParam("heure_demarrage")->value().toCharArray(programme.heure_demarrage,6);  }
   if (request->hasParam("heure_arret")) { request->getParam("heure_arret")->value().toCharArray(programme.heure_arret,6);  }
   if (request->hasParam("temperature")) { programme.temperature = request->getParam("temperature")->value().toInt();  programme.saveProgramme(); }



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
  char buffer[5];
  #ifdef SSR
  int instant_power= sysvar.puissance ; 
  #else
  int instant_power= dimmer.getPower(); 
  #endif

  //state = String(instant_power) + "% " +  String(instant_power * config.charge) + "W"; 
   
  dtostrf(sysvar.celsius,2, 1, buffer); // conversion en n.1f 
  
  DynamicJsonDocument doc(128);
    doc["dimmer"] = instant_power;
    doc["temperature"] = buffer;
    doc["power"] = (instant_power * config.charge/100);
    doc["Ptotal"]  = sysvar.puissance_cumul + (instant_power * config.charge/100);
    // recupération de l'état de surchauffe
    doc["alerte"]  = security;
  serializeJson(doc, state);
  return String(state);
}

String textnofiles() {
  String state = "<html><body>Filesystem is not present. <a href='https://ota.apper-solaire.org/firmware/littlefs-dimmer.bin'>download it here</a> <br>and after  <a href='/update'>upload on the ESP here </a></body></html>" ; 
  return String(state);
}

String processor(const String& var){
  // Serial.println(var);
  /*if (var == "STATE"){
    return getState();
  } */
  if (var == "VERSION"){
    // affichage de la version et de l'environnement
    String VERSION_http = String(VERSION) + " " + String(COMPILE_NAME) ; 
    return (VERSION_http);
  } 
  if (var == "NAME"){
    return (dimmername);
  } 
  return ("N/A");
} 


String getconfig() {
  String configweb;  
  DynamicJsonDocument doc(512);  
  //   +  config.mode + ";" + config.SubscribePV + ";" + config.SubscribeTEMP + ";" + config.dimmer_on_off ;
    doc["IDX"] = config.IDX;
    doc["idxtemp"] = config.IDXTemp;
    doc["IDXAlarme"] = config.IDXAlarme;

    doc["maxtemp"] = config.maxtemp;

    doc["startingpow"] = config.startingpow;
    doc["minpow"] = config.minpow;
    doc["maxpow"] = config.maxpow;

    doc["child"] = config.child;
    doc["delester"] = config.mode;

    doc["SubscribePV"] = config.SubscribePV;
    doc["SubscribeTEMP"] = config.SubscribeTEMP;
    doc["dimmer_on_off"] = config.dimmer_on_off;
    doc["charge"] = config.charge;
  
  serializeJson(doc, configweb);
  return String(configweb);
}
/*
String getminuteur(Programme name) {
    String retour;
    DynamicJsonDocument doc(128); 

    doc["heure_demarrage"] = name.heure_demarrage;
    doc["heure_arret"] = name.heure_arret;
    doc["temperature"] = name.temperature;
    doc["heure"] = timeClient.getHours();
    doc["minute"] = timeClient.getMinutes();

  serializeJson(doc, retour);
  return String(retour) ;
} */ 

String getMinuteur(const Programme& minuteur) {
    DynamicJsonDocument doc(128);
    doc["heure_demarrage"] = minuteur.heure_demarrage;
    doc["heure_arret"] = minuteur.heure_arret;
    doc["temperature"] = minuteur.temperature;
    doc["heure"] = timeClient.getHours();
    doc["minute"] = timeClient.getMinutes();

    String retour;
    serializeJson(doc, retour);
    return retour;
}


String getmqtt() {
    String retour;
  DynamicJsonDocument doc(512); 

    doc["server"] = config.hostname;
    doc["port"] = config.port;
    doc["topic"] = config.Publish;
    doc["user"] = mqtt_config.username;
    doc["password"] = mqtt_config.password;
    doc["MQTT"] = mqtt_config.mqtt;
  serializeJson(doc, retour);
  return String(retour) ;
}

String getcomplement() {
  String retour;
  DynamicJsonDocument doc(64); 

    doc["hdebut"] = config.hostname;
    doc["hfin"] = config.port;
    doc["tmax"] = config.maxtemp;

  serializeJson(doc, retour);
  return String(retour) ;
}


String readmqttsave(){
        String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
        String node_id = String("dimmer-") + node_mac; 
        String save_command = String("Xlyric/sauvegarde/"+ node_id );
        client.subscribe(save_command.c_str(),1);
        return String("<html><head><meta http-equiv='refresh' content='5;url=config.html' /></head><body><h1>config restauree, retour au setup dans 5 secondes, pensez a sauvegarder sur la flash </h1></body></html>");
}

String getServermode(String Servermode) {
  if ( Servermode == "MQTT" ) {   mqtt_config.mqtt = !mqtt_config.mqtt; }
  if ( Servermode == "HA" ) {   mqtt_config.HA = !mqtt_config.HA; }
  if ( Servermode == "jeedom" ) {   mqtt_config.jeedom = !mqtt_config.jeedom; }
  if ( Servermode == "domoticz" ) {   mqtt_config.domoticz = !mqtt_config.domoticz; }
return String(Servermode);
}

#endif