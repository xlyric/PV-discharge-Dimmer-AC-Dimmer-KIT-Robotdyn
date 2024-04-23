#ifndef WEB_FUNCTIONS
#define WEB_FUNCTIONS

// #include <ESP8266WiFi.h>
//#include <ESPAsyncWiFiManager.h> 
#include <stdlib.h>

// #include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
// #include <ESP8266HTTPClient.h> 
#include "config/config.h"
#include "function/littlefs.h"
#include "function/ha.h"
#include "function/minuteur.h"
//#include "function/jotta.h"
#include "function/unified_dimmer.h"

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
extern gestion_puissance unified_dimmer; 

//extern dimmerLamp dimmer; 
extern DNSServer dns;
extern byte security; 

AsyncWebServer server(80);

AsyncWiFiManager wifiManager(&server,&dns);



extern bool AP; 

extern HA device_dimmer; 
extern HA device_temp[MAX_DALLAS]; 
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

extern DeviceAddress addr[MAX_DALLAS];

const char* PARAM_INPUT_1 = "POWER"; /// paramettre de retour sendmode
const char* PARAM_INPUT_2 = "OFFSET"; /// paramettre de retour sendmode

//extern char buffer[1024];

String getmqtt(); 
String getconfig(); 
String getState();
String getState_dallas();
String textnofiles();
String processor(const String& var);
String getServermode(String Servermode);

String switchstate(int state);
String readmqttsave();
String getMinuteur(const Programme& minuteur);
String getMinuteur();
extern Logs Logging; 
extern String devAddrNames[MAX_DALLAS];

#ifdef SSR_ZC
extern SSR_BURST ssr_burst;
#endif


void call_pages() {

/// page de index et récupération des requetes de puissance
  server.on("/",HTTP_ANY, [](AsyncWebServerRequest *request){
    
    if  (LittleFS.exists("/index.html")) {
      DEBUG_PRINTLN(("%d------------------",__LINE__));
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

        float input=request->getParam(PARAM_INPUT_1)->value().toFloat();

        
          if (input==0) {
            sysvar.puissance = 0 ;            // En %
            //int dispo=0;                          // En % // on ne s'en sert pas, donc commenté
            sysvar.puissance_dispo = 0;         // En W
            sysvar.change = 0; // par sécurité, au cas ou le main n'aurait pas fini
          }
                  
        // Modif RV - correction bug si dimmer configuré mais pas allumé ou planté
          else if (sysvar.change == 1){}
                      
/// si remontée de puissance dispo alors prioritaire
          else if (request->hasParam("puissance")) {
/// on recupère la puissance disponible  
            float dispo = request->getParam("puissance")->value().toFloat();

            DEBUG_PRINTLN("puissance="+String(dispo));
            /// on la converti en pourcentage de charge et config.dispo contient la puissance disponible en W 
            config.dispo = dispo;               // En W
            sysvar.puissance_dispo = dispo;     // En W
            dispo = (100*dispo/config.charge);  // En %
 
            
              /// si on dépasse le max, on calcule la puissance dispo restante pour un dimmer enfant
              if (unified_dimmer.get_power() + dispo >= config.maxpow ) {       // En %
                sysvar.puissance_dispo = (config.dispo - ((config.maxpow - unified_dimmer.get_power()) * config.charge / 100));  // En W
              }

            // on égalise
            if ( strcmp(config.child,"") != 0 && strcmp(config.child,"none") != 0 && strcmp(config.mode,"equal") == 0  ) {
              if ( (security == 1) || (unified_dimmer.get_power() >= config.maxpow) ) {
                sysvar.puissance = sysvar.puissance + dispo;          // En %
                sysvar.puissance_dispo = sysvar.puissance_dispo * 2 ; //  En W - On multiplie par 2 car la fonction child_communication() fera / 2
                              }
              else { // Mode avec dimmer enfant en equal ET toujours en capacité de router localement de la puissance
              sysvar.puissance = sysvar.puissance + dispo/2;        // En %
                  }
            }
            else {  // si mode sans dimmer enfant ou en mode délestage vers un fils
              sysvar.puissance = sysvar.puissance + dispo;            // En %
                              }
         }
         
         else // uniquement la commande /?POWER= reçue
         {
           sysvar.puissance = input;
                      sysvar.puissance_dispo = 0;
                      DEBUG_PRINTLN(("%d-input=" + String(input),__LINE__));
         }

        // si config.child = 0.0.0.0 alors max = 100 
        int max = 200;
        if (strcmp(config.child,"none") == 0 || strcmp(config.mode,"off") == 0 ) { max = 100; } 
        if (sysvar.puissance >= max) {sysvar.puissance = max; }
        logging.Set_log_init("HTTP power at " + String(sysvar.puissance) + "%\r\n");
// Modif RV - correction bug si dimmer configuré mais pas allumé ou planté
        if (sysvar.change == 0) { 
        sysvar.change=1; 
        }
        String pb=getState().c_str(); 
        // modif faite le 11 Juin 2023, pourquoi ? pas trouvé l'utilité, log ?
        // pb = pb +String(sysvar.puissance) +" " + String(input) +" " + String(sysvar.puissance_dispo) ;
        request->send_P(200, "text/plain", pb.c_str() );  
      } 
      
              else if (request->hasParam(PARAM_INPUT_2)) { 
        config.startingpow = request->getParam(PARAM_INPUT_2)->value().toInt(); 
        logging.Set_log_init("HTTP power at " + String(config.startingpow)+"W\r\n");
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
    DEBUG_PRINTLN(("%d------------------",__LINE__));
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

    server.on("/state_dallas", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getState_dallas().c_str());
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
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/css/all.css.gz", "text/css");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });


  server.on("/all.css", HTTP_ANY, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/css/all.css.gz", "text/css");
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

  server.on("/programme.json", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/programme.json", "application/json");
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


  server.on("/getminiteur", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("dimmer")) { request->send(200, "application/json",  getMinuteur(programme));  }
    if (request->hasParam("relay1")) { request->send(200, "application/json",  getMinuteur(programme_relay1)); }
    if (request->hasParam("relay2")) { request->send(200, "application/json",  getMinuteur(programme_relay2)); }
    else { request->send(200, "application/json",  getMinuteur()); }
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

  /// reglage des seuils relais
    server.on("/relai.html", HTTP_ANY, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/relai.html", "text/html");
    //response->addHeader("Content-Encoding", "gzip");
    request->send(response);
   // request->send(LittleFS, "/mqtt.html", "text/html");
  });

    server.on("/getseuil", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("relay1")) { request->send(200, "application/json",  getMinuteur(programme_relay1)); }
    if (request->hasParam("relay2")) { request->send(200, "application/json",  getMinuteur(programme_relay2)); }
    //request->send(200, "application/json",  getminuteur(programme_relay2).c_str()); 
  });

  server.on("/setseuil", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    String name; 
    if (request->hasParam("relay1")) { 
            if (request->hasParam("seuil_demarrage")) { programme_relay1.seuil_start =request->getParam("seuil_demarrage")->value().toInt();   }
            if (request->hasParam("seuil_arret")) { programme_relay1.seuil_stop =request->getParam("seuil_arret")->value().toInt();   }
            if (request->hasParam("temperature")) { programme_relay1.seuil_temperature = request->getParam("temperature")->value().toInt();  programme_relay1.saveProgramme(); }
      request->send(200, "application/json",  getMinuteur(programme_relay1)); 
    }
    if (request->hasParam("relay2")) { 
            if (request->hasParam("seuil_demarrage")) { programme_relay2.seuil_start = request->getParam("seuil_demarrage")->value().toInt();   }
            if (request->hasParam("seuil_arret")) { programme_relay2.seuil_stop = request->getParam("seuil_arret")->value().toInt();   }
            if (request->hasParam("temperature")) { programme_relay2.seuil_temperature = request->getParam("temperature")->value().toInt();  programme_relay2.saveProgramme(); }
      request->send(200, "application/json",  getMinuteur(programme_relay2)); 
    }
  });



  server.on("/config", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getconfig().c_str());
  });

  server.on("/reset", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain","Restarting");
    // ESP.restart();
    config.restart = true;
  });

  server.on("/cs", HTTP_ANY, [](AsyncWebServerRequest *request){
    logging.Set_log_init("}1");
    request->send_P(200, "text/plain", logging.Get_log_init().c_str());
    // reinit de logging.log_init 
    logging.reset_log_init(); 
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
   //if (request->hasParam("charge")) { config.charge = request->getParam("charge")->value().toInt();}
if (request->hasParam("charge1")) { 
    config.charge1 = request->getParam("charge1")->value().toInt(); 
    config.charge = config.charge1 + config.charge2 + config.charge3;
    if (!AP && mqtt_config.mqtt) { //device_dimmer_charge.send(String(config.charge));
      }
    }
   if (request->hasParam("charge2")) { 
    config.charge2 = request->getParam("charge2")->value().toInt(); 
    config.charge = config.charge1 + config.charge2 + config.charge3;
    if (!AP && mqtt_config.mqtt) {  //device_dimmer_charge.send(String(config.charge));
      }
    }
   if (request->hasParam("charge3")) { 
    config.charge3 = request->getParam("charge3")->value().toInt(); 
    config.charge = config.charge1 + config.charge2 + config.charge3;
    if (!AP && mqtt_config.mqtt) { //device_dimmer_charge.send(String(config.charge));
      }
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
/*
    if (request->hasParam("charge")) { 
    config.charge = request->getParam("charge")->value().toInt();
    if (!AP && mqtt_config.mqtt) { device_dimmer_charge.send(String(config.charge));}
   }
*/
   if (request->hasParam("child")) { request->getParam("child")->value().toCharArray(config.child,15);  }
   if (request->hasParam("mode")) { 
    request->getParam("mode")->value().toCharArray(config.mode,10);  
    if (!AP && mqtt_config.mqtt) { device_dimmer_child_mode.send(String(config.mode));}
   }

   if (request->hasParam("dimmername")) { request->getParam("dimmername")->value().toCharArray(config.say_my_name,100);}
   if (request->hasParam("SubscribePV")) { request->getParam("SubscribePV")->value().toCharArray(config.SubscribePV,100);}
   if (request->hasParam("SubscribeTEMP")) { request->getParam("SubscribeTEMP")->value().toCharArray(config.SubscribeTEMP,100);}
   if (request->hasParam("dimmer_on_off")) { 
    config.dimmer_on_off = request->getParam("dimmer_on_off")->value().toInt();
    if (!AP && mqtt_config.mqtt) { device_dimmer_on_off.send(String(config.dimmer_on_off));}
   }
   if (request->hasParam("mqttuser")) { request->getParam("mqttuser")->value().toCharArray(mqtt_config.username,50);  }
   if (request->hasParam("mqttpassword")) { 
    request->getParam("mqttpassword")->value().toCharArray(mqtt_config.password,50);
    savemqtt(mqtt_conf, mqtt_config); 
    saveConfiguration(filename_conf, config);
   }
   if (request->hasParam("DALLAS")) { 
    request->getParam("DALLAS")->value().toCharArray(config.DALLAS,17); 
    // application de la modification sur la dallas master si existante
    for (int i = 0; i < MAX_DALLAS; i++) {
      if (strcmp(config.DALLAS,(devAddrNames[i]).c_str() ) == 0) { sysvar.dallas_maitre = i; }
    
    }
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

String getState_dallas() {
  String state; 
  char buffer[5];
   
  dtostrf(sysvar.celsius[sysvar.dallas_maitre],2, 1, buffer); // conversion en n.1f 
  
  DynamicJsonDocument doc(384);
    doc["temperature"] = buffer;
    
    //affichage des température et adresse des sondes dallas 
    for (int i = 0; i < MAX_DALLAS; i++) {
      char buffer[5];
      // affichage que si != 0 
      if (sysvar.celsius[i] != 0) {
        dtostrf(sysvar.celsius[i],2, 1, buffer); // conversion en n.1f 
        doc["dallas"+String(i)] = buffer;
        doc["addr"+String(i)] = devAddrNames[i];
      }
    }
  serializeJson(doc, state);
  return String(state);
}


String getState() {
  String state; 
  char buffer[5];
  #ifdef  SSR
    #ifdef SSR_ZC
      int instant_power= unified_dimmer.get_power(); 
    #else
      int instant_power= sysvar.puissance ;
    #endif
  #else
  float instant_power= unified_dimmer.get_power(); 
  #endif

  //state = String(instant_power) + "% " +  String(instant_power * config.charge) + "W"; 
   
  dtostrf(sysvar.celsius[sysvar.dallas_maitre],2, 1, buffer); // conversion en n.1f 
  
  DynamicJsonDocument doc(384);
    doc["dimmer"] = int(instant_power); // on le repasse un int pour éviter un affichage trop grand
    doc["commande"] = int(sysvar.puissance);
    doc["temperature"] = buffer;
    doc["power"] = int(instant_power * config.charge/100);
    doc["Ptotal"]  = sysvar.puissance_cumul + int(instant_power * config.charge/100);
    // recupération de l'état de surchauffe
    doc["alerte"]  = security;
#ifdef RELAY1    
    doc["relay1"]   = digitalRead(RELAY1);
    doc["relay2"]   = digitalRead(RELAY2);
#else
    doc["relay1"]   = 0;
    doc["relay2"]   = 0;
#endif
    doc["minuteur"] = programme.run;
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
    return (config.say_my_name);
  } 
  if (var == "RSSI"){
    return (String(WiFi.RSSI()));
  } 
  return ("N/A");
} 


String getconfig() {
  String configweb;  
  DynamicJsonDocument doc(512);  
  //   +  config.mode + ";" + config.SubscribePV + ";" + config.SubscribeTEMP + ";" + config.dimmer_on_off ;
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
    doc["DALLAS"] = config.DALLAS;
    doc["dimmername"] = config.say_my_name;
    doc["charge1"] = config.charge1;
    doc["charge2"] = config.charge2;
    doc["charge3"] = config.charge3;
  
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

String getMinuteur(const Programme& minuteur ) {
    getLocalTime(&timeinfo);
    DynamicJsonDocument doc(256);
    doc["heure_demarrage"] = minuteur.heure_demarrage;
    doc["heure_arret"] = minuteur.heure_arret;
    doc["temperature"] = minuteur.temperature;
    doc["heure"] = timeinfo.tm_hour;
    doc["minute"] = timeinfo.tm_min;
    doc["seuil_start"] = minuteur.seuil_start;
    doc["seuil_stop"] = minuteur.seuil_stop;
    doc["seuil_temp"] = minuteur.seuil_temperature;

    String retour;
    serializeJson(doc, retour);
    return retour;
}

String getMinuteur() {
    getLocalTime(&timeinfo);
    DynamicJsonDocument doc(256);
    doc["heure"] = timeinfo.tm_hour;
    doc["minute"] = timeinfo.tm_min;
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
    doc["HA"] = config.HA;
    doc["JEEDOM"] = config.JEEDOM;
    doc["DOMOTICZ"] = config.DOMOTICZ;
    doc["IDX"] = config.IDX;
    doc["idxtemp"] = config.IDXTemp;
    doc["IDXAlarme"] = config.IDXAlarme;
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
        //String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
        //String node_id = String("dimmer-") + node_mac; 
        String node_id = config.say_my_name;
        String save_command = String("Xlyric/sauvegarde/"+ node_id );
        client.subscribe(save_command.c_str(),1);
        return String("<html><head><meta http-equiv='refresh' content='5;url=config.html' /></head><body><h1>config restauree, retour au setup dans 5 secondes, pensez a sauvegarder sur la flash </h1></body></html>");
}

String getServermode(String Servermode) {
  if ( Servermode == "MQTT" ) {   mqtt_config.mqtt = !mqtt_config.mqtt; }
  if ( Servermode == "HA" ) {   config.HA = !config.HA; }
  if ( Servermode == "JEEDOM" ) {   config.JEEDOM = !config.JEEDOM; }
  if ( Servermode == "DOMOTICZ" ) {   config.DOMOTICZ = !config.DOMOTICZ; }

return String(Servermode);
}
#endif