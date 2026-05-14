#ifndef WEB_FUNCTIONS
#define WEB_FUNCTIONS

#include <stdlib.h>

#include <ESPAsyncWebServer.h>
#include "config/config.h"
#include "function/littlefs.h"
#include "function/ha.h"
#include "function/minuteur.h"
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

#include "StreamConcat.h"
extern bool boost();


extern Mqtt mqtt_config;
extern Config config;
extern System sysvar;
extern Programme programme;
extern Programme programme_relay1;
extern Programme programme_relay2;
extern Programme programme_marche_forcee;
extern gestion_puissance unified_dimmer;

extern DNSServer dns;

AsyncWebServer server(80);

AsyncWiFiManager wifiManager(&server,&dns);



extern bool AP;

extern HA device_dimmer;
extern HA device_temp[MAX_DALLAS];
extern HA device_relay1;
extern HA device_relay2;
extern HA device_dimmer_on_off;
extern HA device_dimmer_child_mode;
extern HA device_dimmer_maxpow;
extern HA device_dimmer_minpow;
extern HA device_dimmer_starting_pow;
extern HA device_dimmer_maxtemp;


extern String dimmername;

extern DeviceAddress addr[MAX_DALLAS];

constexpr const char* PARAM_INPUT_1 = "POWER";  /// paramettre de retour sendmode
constexpr const char* PARAM_INPUT_2 = "OFFSET"; /// paramettre de retour sendmode


String getmqtt();
String getconfig();
String getState();
String getState_dallas();

String textnofiles();
//String processor(const String& var);
String getServermode(String Servermode);

String switchstate(int state);
String readmqttsave();
String getMinuteur(const Programme& minuteur);
String getMinuteur();
String replaceSpacesWithHyphens(String input);

bool checkAuth(AsyncWebServerRequest *request);

extern Logs Logging;
extern String devAddrNames[MAX_DALLAS];
extern bool otaRequested;

#ifdef SSR_ZC
extern SSR_BURST ssr_burst;
#endif

bool checkAuth(AsyncWebServerRequest *request) {
    if (!config.auth_enabled) return true;
    if (request->authenticate("admin", config.auth_pass.c_str())) return true;
    request->requestAuthentication("PV Dimmer");
    return false;
  }

void call_pages() {

  /*
      {"/css/fa-solid-900.woff2", "/css/fa-solid-900.woff2"},
    {"/favicon.ico", "/favicon.ico"},
    ,
    {"/style.css", "/style.css"},
    {"/script.js", "/script.js"}
  */
  // Static pages, see compressed
  const char* staticFiles[][2] = {
   /* {"/js/all.min.js", "/js/all.min.js"},
    {"/css/all.min.css", "/css/all.min.css"},
    {"/log.html", "/log.html"},
    {"/mqtt.html", "/mqtt.html"},
    {"/minuteur.html", "/minuteur.html"},
    {"/relai.html", "/relai.html"},
    {"/backup.html", "/backup.html"}
    {"/lang.json", "/lang.json"}, */
    {"/js/app.js", "/js/app.js"},
    {"/css/style.css", "/css/style.css"},
    {"/icons.svg", "/icons.svg"},
    {"/js/i18n.js" , "/js/i18n.js"}

  };

  for (const auto& file : staticFiles) {
   /*if (strstr(file[0], ".html")) {
      server.serveStatic(file[0], LittleFS, file[1]).setTemplateProcessor(processor).setCacheControl("max-age=31536000");
    } else {*/
      server.serveStatic(file[0], LittleFS, file[1]).setCacheControl("max-age=31536000");
    //}
  }

  // page config.html
 /* if (!AP) {
    server.serveStatic("/config.html", LittleFS, "/config.html").setTemplateProcessor(processor).setCacheControl("max-age=31536000");
  } else {
    server.serveStatic("/config-AP.html", LittleFS, "/config-AP.html").setTemplateProcessor(processor).setCacheControl("max-age=31536000");
  }*/

  // page de index et récupération des requetes de puissance
  server.on("/",HTTP_ANY, [](AsyncWebServerRequest *request){
    // protection contre la puissance négative
    if (sysvar.puissance < 0 ) { sysvar.puissance = 0; } // on ne peut pas être en dessous de 0

    sysvar.lock_mqtt=true;  // on bloque les requetes MQTT 
    if  (LittleFS.exists("/index.html")) {
      DEBUG_PRINTLN(("%d------------------",__LINE__));
      DEBUG_PRINTLN(sysvar.puissance);
      // si requete sur POWER

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
        

        if (input<=0) {
          sysvar.puissance = 0;         // En %
          sysvar.puissance_dispo = 0;   // En W
          sysvar.change = 0;            // par sécurité, au cas ou le main n'aurait pas fini
        }

        /// si remontée de puissance dispo alors prioritaire
        else if (request->hasParam("puissance")) {
          /// on recupère la puissance disponible
          float dispo = request->getParam("puissance")->value().toFloat();

          DEBUG_PRINTLN("puissance="+String(dispo));
          /// on la converti en pourcentage de charge et config.dispo contient la puissance disponible en W
          config.dispo = dispo;                 // En W
          sysvar.puissance_dispo = dispo;       // En W
          dispo = (100*dispo/config.charge);    // En %

          /// si on dépasse le max, on calcule la puissance dispo restante pour un dimmer enfant
          if (unified_dimmer.get_power() + dispo >= config.maxpow ) {           // En %
            sysvar.puissance_dispo =
              (config.dispo - ((config.maxpow - unified_dimmer.get_power()) * config.charge / 100));    // En W
          }

          // on égalise
          if ( strcmp(config.child,"") != 0 && strcmp(config.child,"none") != 0 && strcmp(config.mode,"equal") == 0  ) {
            if ( (sysvar.security ) || (unified_dimmer.get_power() >= config.maxpow) ) {
              sysvar.puissance = sysvar.puissance + dispo;            // En %
              sysvar.puissance_dispo = sysvar.puissance_dispo * 2;    // En W - On multiplie par 2 car la fonction child_communication() fera / 2
            }
            else {  // Mode avec dimmer enfant en equal ET toujours en capacité de router localement de la puissance
              sysvar.puissance = sysvar.puissance + dispo/2;        // En %
            }
          }
          else {  // si mode sans dimmer enfant ou en mode délestage vers un fils
            sysvar.puissance = sysvar.puissance + dispo;              // En %
          }
        }

        else {  // uniquement la commande /?POWER= reçue
          sysvar.puissance = input;
          sysvar.puissance_dispo = 0;
          DEBUG_PRINTLN(("%d-input=" + String(input),__LINE__));
        }

        // si config.child = 0.0.0.0 alors max = 100
        int max = 200;
        if (strcmp(config.child,"none") == 0 || strcmp(config.mode,"off") == 0 ) { max = 100; }
        if (sysvar.puissance >= max) {sysvar.puissance = max; }
        char temp_buffer[128]; // Ajustez la taille en fonction de la longueur maximale attendue
        float temp_puissance_affiche = sysvar.puissance;
        snprintf(temp_buffer, sizeof(temp_buffer),  "%s %.2f%%\r\n",HTTP_power_at, temp_puissance_affiche); //NOSONAR: formatage intentionnel pour éviter les problèmes de mémoire avec les String
        logging.Set_log_init(temp_buffer);
        // Modif RV - correction bug si dimmer configuré mais pas allumé ou planté
        if (sysvar.change == 0) {
          sysvar.change=1;
        }
        String pb=getState().c_str();
        request->send(200, "application/json", pb.c_str() );
      }

      else if (request->hasParam(PARAM_INPUT_2)) {
        config.startingpow = request->getParam(PARAM_INPUT_2)->value().toInt();
        char temp_buffer[128]; // Ajustez la taille en fonction de la longueur maximale attendue
        snprintf(temp_buffer, sizeof(temp_buffer),  "%s %dW\r\n",HTTP_power_at, config.startingpow);
        logging.Set_log_init(temp_buffer);
        sysvar.change=1;
        request->send(200, "application/json", getState().c_str());
      }

      else  {
        //if (!AP) 
        {
          AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html", String(), false, nullptr);
          response->addHeader("cache-control","max-age=31536000");
          request->send(response);

          //request->send(LittleFS, "/index.html", String(), false, processor);
        } /*else {
          AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index-AP.html", String(), false, processor);
          response->addHeader("cache-control","max-age=31536000");
          request->send(response);
          //request->send(LittleFS, "/index-AP.html", String(), false, processor);
        }*/
      }
    }
    else
    {
      request->send(200, "text/html", textnofiles().c_str());
    }

    DEBUG_PRINTLN(sysvar.puissance);
    DEBUG_PRINTLN(("%d------------------",__LINE__));
    sysvar.lock_mqtt=false; // on débloque les requetes MQTT
  });

  server.on("/state", HTTP_ANY, [](AsyncWebServerRequest *request){
    sysvar.lock_mqtt=true;  // on bloque les requetes MQTT
    request->send(200, "application/json", getState().c_str());
    sysvar.lock_mqtt=false; // on débloque les requetes MQTT
  });

  server.on("/state_dallas", HTTP_ANY, [](AsyncWebServerRequest *request){
    sysvar.lock_mqtt=true;  // on bloque les requetes MQTT
    request->send(200, "application/json", getState_dallas().c_str());
    sysvar.lock_mqtt=false; // on débloque les requetes MQTT
  });

  server.on("/config", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getconfig().c_str());
  });

  
  server.on("/getmqtt", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    request->send(200, "application/json",  getmqtt().c_str());
  });

  server.on("/resetwifi", HTTP_ANY, [](AsyncWebServerRequest *request){
    if (!checkAuth(request)) return;
    request->send(200, "text/plain", "Resetting Wifi and reboot");
    wifiManager.resetSettings();
    config.restart = true;
  });

  server.on("/reboot", HTTP_ANY, [](AsyncWebServerRequest *request){
    if (!checkAuth(request)) return;
    request->redirect("/");
    config.restart = true;
  });

  server.on("/disconnect", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->redirect("/");
    WiFi.disconnect();
  });

  server.on("/ping", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "pong");
  });

  const char* jsonFiles[][2] = {
    {"/config.json"},
    {"/mqtt.json"},
    {"/wifi.json"},
    {"/programme.json"}
  };

  for (const auto& file : jsonFiles) {
    server.serveStatic(file[0], LittleFS, file[0]);
  }

  // ajout de la commande de boost 2h   
  server.on("/boost", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    boost();    
    request->send(200, "application/json",  getMinuteur(programme_marche_forcee));
  });

/// service OTA pour vérifier la version du firmware
    server.on("/otacheck", HTTP_GET, [](AsyncWebServerRequest *request) {
      HTTPClient http;
      #ifdef ARDUINO_ARCH_ESP8266
        WiFiClient client;
        http.begin(client, OTA_JSON);
      #else
        http.begin(OTA_JSON);
      #endif

      int code = http.GET();
      if (code == 200) {
        request->send(200, "application/json", http.getString());
      } else {
        request->send(502, "application/json", "{\"error\":\"upstream failed\"}");
      }
      http.end();
    });

    server.on("/otaupdate", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"status\":\"Update started\"}");
         otaRequested = true;
    });

  server.on("/getminuteur", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("dimmer")) { request->send(200, "application/json",  getMinuteur(programme)); }
    else if (request->hasParam("relay1")) { request->send(200, "application/json",  getMinuteur(programme_relay1)); }
    else if (request->hasParam("relay2")) { request->send(200, "application/json",  getMinuteur(programme_relay2)); }
    else { request->send(200, "application/json",  getMinuteur()); }
  });

  server.on("/setminuteur", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    String name;
    if (request->hasParam("dimmer")) {
      if (request->hasParam("heure_demarrage")) {
        request->getParam("heure_demarrage")->value().toCharArray(
          programme.heure_demarrage, 6);
      }
      if (request->hasParam("heure_arret")) {
        request->getParam("heure_arret")->value().toCharArray(
          programme.heure_arret,6);
      }
      if (request->hasParam("puissance")) {
        programme.puissance = request->getParam("puissance")->value().toInt();
      }
      if (request->hasParam("temperature")) {
        programme.temperature = request->getParam(
          "temperature")->value().toInt();
        programme.saveProgramme();
      }
      request->send(200, "application/json",  getMinuteur(programme));
    }
    if (request->hasParam("relay1")) {
      if (request->hasParam("heure_demarrage")) {
        request->getParam("heure_demarrage")->value().toCharArray(
          programme_relay1.heure_demarrage,6);
      }
      if (request->hasParam("heure_arret")) {
        request->getParam("heure_arret")->value().toCharArray(
          programme_relay1.heure_arret,6);
      }
      if (request->hasParam("temperature")) {
        programme_relay1.temperature = request->getParam(
          "temperature")->value().toInt();
        programme_relay1.saveProgramme();
      }
      request->send(200, "application/json", getMinuteur(programme_relay1));
    }
    if (request->hasParam("relay2")) {
      if (request->hasParam("heure_demarrage")) {
        request->getParam("heure_demarrage")->value().toCharArray(
          programme_relay2.heure_demarrage,6);
      }
      if (request->hasParam("heure_arret")) {
        request->getParam("heure_arret")->value().toCharArray(
          programme_relay2.heure_arret,6);
      }
      if (request->hasParam("temperature")) {
        programme_relay2.temperature = request->getParam(
          "temperature")->value().toInt();  programme_relay2.saveProgramme();
      }
      request->send(200, "application/json",  getMinuteur(programme_relay2));
    }
    else { request->send(200, "application/json",  getMinuteur()); }
  });

  /// reglage des seuils relais
  server.on("/getseuil", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("relay1")) { request->send(200, "application/json",  getMinuteur(programme_relay1)); }
    else if (request->hasParam("relay2")) { request->send(200, "application/json",  getMinuteur(programme_relay2)); }
    else { request->send(200, "application/json",  getMinuteur()); }
  });

  server.on("/setseuil", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    String name;
    if (request->hasParam("relay1")) {
      if (request->hasParam("seuil_demarrage")) {
        programme_relay1.seuil_start =request->getParam(
          "seuil_demarrage")->value().toInt();
      }
      if (request->hasParam("seuil_arret")) {
        programme_relay1.seuil_stop =request->getParam(
          "seuil_arret")->value().toInt();
      }
      if (request->hasParam("temperature")) {
        programme_relay1.seuil_temperature = request->getParam(
          "temperature")->value().toInt();  programme_relay1.saveProgramme();
      }
      request->send(200, "application/json",  getMinuteur(programme_relay1));
    }
    if (request->hasParam("relay2")) {
      if (request->hasParam("seuil_demarrage")) {
        programme_relay2.seuil_start = request->getParam(
          "seuil_demarrage")->value().toInt();
      }
      if (request->hasParam("seuil_arret")) {
        programme_relay2.seuil_stop = request->getParam(
          "seuil_arret")->value().toInt();
      }
      if (request->hasParam("temperature")) {
        programme_relay2.seuil_temperature = request->getParam(
          "temperature")->value().toInt();  programme_relay2.saveProgramme();
      }
      request->send(200, "application/json",  getMinuteur(programme_relay2));
    }
    else { request->send(200, "application/json",  getMinuteur()); }
  });


  server.on("/reset", HTTP_ANY, [](AsyncWebServerRequest *request){
    if (!checkAuth(request)) return;
    // faire un redirect vers / 
    request->redirect("/");
    config.restart = true;
  });

  server.on("/cs", HTTP_ANY, [](AsyncWebServerRequest *request){
    logging.Set_log_init("}1");
    request->send(200, "text/plain", logging.Get_log_init().c_str());
    // reinit de logging.log_init
    logging.reset_log_init();
  });

  server.on("/onoff", HTTP_ANY, [](AsyncWebServerRequest *request){
    getServermode("ONOFF");
    request->send(200, "text/html", config.dimmer_on_off ? "1" : "0");
  });

  server.on("/save", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/config.json", String(), true);
  });

  server.on("/readmqtt", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", readmqttsave().c_str());
  });

  // ── GET /getauth ─────────────────────────────────────────────────
  server.on("/getauth", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    String json = "{\"auth_enabled\":" + String(config.auth_enabled ? "true" : "false") + "}";
    request->send(200, "application/json", json);
  });

  // ── POST /setauth ────────────────────────────────────────────────
  server.on("/setauth", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;

    auto has = [&](const char* n) {
      return request->hasParam(n, true) || request->hasParam(n);
    };
    auto val = [&](const char* n) -> String {
      if (request->hasParam(n, true)) return request->getParam(n, true)->value();
      if (request->hasParam(n))       return request->getParam(n)->value();
      return "";
    };

    if (has("auth_enabled")) {
      config.auth_enabled = (val("auth_enabled") == "1");
    }
    if (has("auth_pass") && val("auth_pass").length() > 0) {
      config.auth_pass = val("auth_pass");
    }
    
    config.saveConfiguration().c_str();
    
    request->send(200, "application/json", "{\"ok\":true}");
  });

/////////////////////////
////// mise à jour parametre d'envoie vers domoticz et récupération des modifications de configurations
/////////////////////////

  server.on("/get", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      if (!checkAuth(request)) return;
    // Ajouter ces deux lambdas en tête du handler
    auto has = [&](const char* n) {
      return request->hasParam(n, true) || request->hasParam(n);
    };
    auto val = [&](const char* n) -> String {
      if (request->hasParam(n, true)) return request->getParam(n, true)->value();
      if (request->hasParam(n))       return request->getParam(n)->value();
      return "";
    };

    ///  fonction  /get?paramettre=xxxx
    if (has("save")) {
      Serial.println(F("Saving configuration..."));
      logging.Set_log_init(config.saveConfiguration().c_str()); // sauvegarde de la configuration
    }

    if (has("hostname")) { val("hostname").toCharArray(config.hostname,16); }
    if (has("port")) { config.port = val("port").toInt();  }
    if (has("Publish")) { val("Publish").toCharArray(config.Publish,100); }
    if (has("idxtemp")) { config.IDXTemp = val("idxtemp").toInt(); }
    if (has("maxtemp")) {
      config.maxtemp = val("maxtemp").toInt();
      if (!AP && mqtt_config.mqtt) { device_dimmer_maxtemp.sendInt(config.maxtemp);}
    }
    if (has("mintemp")) {
      config.mintemp = val("mintemp").toInt();
    }
    if (has("charge1")) {
      config.charge1 = val("charge1").toInt();
      config.charge = config.charge1 + config.charge2 + config.charge3;
    }
    if (has("charge2")) {
      config.charge2 = val("charge2").toInt();
      config.charge = config.charge1 + config.charge2 + config.charge3;
    }
    if (has("charge3")) {
      config.charge3 = val("charge3").toInt();
      config.charge = config.charge1 + config.charge2 + config.charge3;
    }
    if (has("IDXAlarme")) { config.IDXAlarme = val("IDXAlarme").toInt(); }
    if (has("IDX")) { config.IDX = val("IDX").toInt(); }
    if (has("startingpow")) {
      config.startingpow = val("startingpow").toInt();
      if (!AP && mqtt_config.mqtt) {
        device_dimmer_starting_pow.sendInt(config.startingpow);
      }
    }
    if (has("minpow")) {
      config.minpow = val("minpow").toInt();
      if (!AP && mqtt_config.mqtt) { device_dimmer_minpow.sendInt(config.minpow); }
    }
    if (has("maxpow")) {
      config.maxpow = val("maxpow").toInt();
      if (!AP && mqtt_config.mqtt) { device_dimmer_maxpow.sendInt(config.maxpow); }
    }

    if (has("child")) { val("child").toCharArray(config.child,64); }
    if (has("mode")) {
      val("mode").toCharArray(config.mode,10);
      if (!AP && mqtt_config.mqtt) { device_dimmer_child_mode.send(config.mode); }
    }

    if (has("dimmername")) {
      val("dimmername").toCharArray(config.say_my_name,100);
      String temp_dimmer_name = replaceSpacesWithHyphens(config.say_my_name);
      // copie du nom du dimmer dans le nom de l'entité
      temp_dimmer_name.toCharArray(config.say_my_name,100);
    }
    if (has("SubscribePV")) {
      val("SubscribePV").toCharArray(
        config.SubscribePV, 100);
    }
    if (has("SubscribeTEMP")) {
      val("SubscribeTEMP").toCharArray(
        config.SubscribeTEMP, 100);
    }
    if (has("dimmer_on_off")) {
      config.dimmer_on_off = val("dimmer_on_off").toInt();
      if (!AP && mqtt_config.mqtt) { device_dimmer_on_off.sendInt(config.dimmer_on_off);}
    }
    if (has("mqttuser")) { val("mqttuser").toCharArray(mqtt_config.username,50); }
    if (has("mqttpassword")) {
      val("mqttpassword").toCharArray(mqtt_config.password,50);
      logging.Set_log_init(config.saveConfiguration().c_str()); // sauvegarde de la configuration
      logging.Set_log_init(mqtt_config.savemqtt().c_str()); // sauvegarde et récupération de la log MQTT
    }
    if (has("DALLAS")) {
      val("DALLAS").toCharArray(config.DALLAS,17);
      // application de la modification sur la dallas master si existante
      for (int i = 0; i < MAX_DALLAS; i++) {
        if (strcmp(config.DALLAS,(devAddrNames[i]).c_str() ) == 0) { sysvar.dallas_maitre = i; }
      }
    }

    // minuteur
    if (has("heure_demarrage")) {
      val("heure_demarrage").toCharArray(
        programme.heure_demarrage, 6);
    }
    if (has("heure_arret")) {
      val("heure_arret").toCharArray(
        programme.heure_arret, 6);
    }
    if (has("temperature")) {
      programme.temperature = val("temperature").toInt();  programme.saveProgramme();
    }

    // trigger
    if (has("trigger")) { config.trigger = val("trigger").toInt();}


    // Ajout des relais
  #ifdef RELAY1
    if (has("relay1")) {
      int relay = val("relay1").toInt();
      if ( relay == 0) { digitalWrite(RELAY1, HIGH); } // correction bug de démarrage en GPIO 0
      else if (relay == 1) { digitalWrite(RELAY1, LOW); } // correction bug de démarrage en GPIO 0
      else if (relay == 2) { digitalWrite(RELAY1, !digitalRead(RELAY1)); }
      // inversion de l'état relaystate par rapport au relais , correction bug de démarrage en GPIO 0
      int relaystate = !digitalRead(RELAY1);
    
      char str[8];  // NOSONAR
      itoa( relaystate, str, 10 );
      request->send(200, "application/json", str );
      if (!AP && mqtt_config.mqtt) { device_relay1.sendInt(relaystate);}
      return;
    }
  #endif
  #ifdef RELAY2
    if (has("relay2")) {
      int relay = val("relay2").toInt();
      if ( relay == 0) { digitalWrite(RELAY2, LOW); }
      else if (relay == 1) { digitalWrite(RELAY2, HIGH); }
      else if (relay == 2) { digitalWrite(RELAY2, !digitalRead(RELAY2)); }
      int relaystate = digitalRead(RELAY2);
      char str[8];  // NOSONAR
      itoa( relaystate, str, 10 );
      request->send(200, "application/json", str );
      if (!AP && mqtt_config.mqtt) { device_relay2.sendInt(relaystate);}
      return;
    }
  #endif

    //// for check boxs in web pages
    if (has("servermode")) {
      String inputMessage = val("servermode");
      getServermode(inputMessage);
      logging.Set_log_init(config.saveConfiguration().c_str()); // sauvegarde de la configuration
      logging.Set_log_init(mqtt_config.savemqtt().c_str()); // sauvegarde et récupération de la log MQTT
    }

    request->send(200, "application/json", getconfig().c_str());
  });
}



/// @brief Pages de traitement
/// @return

String getState_dallas() {
  String state;
  char buffer[5];// NOSONAR

  dtostrf(sysvar.celsius[sysvar.dallas_maitre],2, 1, buffer); // conversion en n.1f

  JsonDocument doc;
  doc["temperature"] = buffer;

  // affichage des température et adresse des sondes dallas
  for (int i = 0; i < MAX_DALLAS; i++) {
    char buffer[5];  // NOSONAR
    // affichage que si != 0
    if (sysvar.celsius[i] != 0) {
      dtostrf(sysvar.celsius[i],2, 1, buffer);   // conversion en n.1f
      doc["dallas"+String(i)] = buffer;
      doc["addr"+String(i)] = devAddrNames[i];
    }
  }
  serializeJson(doc, state);
  return String(state);
}


String getState() {
  String state;
  char buffer[5];// NOSONAR
  #ifdef  SSR
    #ifdef SSR_ZC
  int instant_power= unified_dimmer.get_power();
    #else
  int instant_power= sysvar.puissance;
    #endif
  #else
  float instant_power= unified_dimmer.get_power();
  #endif

  dtostrf(sysvar.celsius[sysvar.dallas_maitre],2, 1, buffer); // conversion en n.1f

  JsonDocument doc;
  doc["dimmer"] = int(instant_power);   // on le repasse un int pour éviter un affichage trop grand
  doc["commande"] = int(sysvar.puissance);
  doc["temperature"] = buffer;
  doc["power"] = int(instant_power * config.charge/100);
  doc["Ptotal"]  = sysvar.puissance_cumul + int(instant_power * config.charge/100);
  doc["RSSI"] = WiFi.RSSI();
  doc["version"] = String(VERSION);
  #ifdef RELAY1
  doc["relay1"]   = !digitalRead(RELAY1);
  doc["relay2"]   = digitalRead(RELAY2);
#else
  doc["relay1"]   = false;
  doc["relay2"]   = false;
#endif
 
// prévision retrait du mode de prepresseur 
  /* doc["FS_RELEASE"] = FS_RELEASE;
  doc["VERSION"] = VERSION;
  doc["RSSI"] = WiFi.RSSI();
  doc["NAME"] = String(config.say_my_name) + ".local";
*/ 


  doc["minuteur"] = programme.run;
  doc["onoff"] = config.dimmer_on_off;
  doc["alerte"] = logging.alerte_web; //affiche maintenant l'alerte et plus 0 ou 1 pour les alertes
if (programme_marche_forcee.run) {
  doc["boost"] = programme_marche_forcee.run;
  doc["boost_endtime"] = programme_marche_forcee.heure_arret; 
  } 
  doc["boost_max_temp"] = programme_marche_forcee.temperature;
    // affichage des température et adresse des sondes dallas
  for (int i = 0; i < MAX_DALLAS; i++) {
    char buffer[5];  // NOSONAR
    // affichage que si != 0
    if (sysvar.celsius[i] != 0) {
      dtostrf(sysvar.celsius[i],2, 1, buffer);   // conversion en n.1f
      doc["dallas"+String(i)] = buffer;
      doc["addr"+String(i)] = devAddrNames[i];
    }
  }
  serializeJson(doc, state);
  return String(state);
}

String textnofiles() {
  String state = "<html><head><meta http-equiv='refresh' content='10'></head><body>Filesystem is not present.<a href='https://ota.apper-solaire.org/firmware/littlefs-dimmer.bin'>download it here</a><br>and after <a href='/update'>upload on the ESP here</a><br>or <a href='/reset'>reboot </a></body></html>";
  return String(state);
}

 /*String processor(const String& var){

  if (var == "VERSION") {
    // affichage de la version et de l'environnement
    auto VERSION_http = String(VERSION) + " " + String(COMPILE_NAME);
    return VERSION_http;
  }
  if (var == "NAME") {
    auto name = String(config.say_my_name) + ".local";
    return name;
  }
  if (var == "RSSI") {
    return (String(WiFi.RSSI()));
  }
  if (var == "FS_RELEASE") {
    return String(FS_RELEASE);
  } 
  return "";
} */


String getconfig() {
  String configweb;
  JsonDocument doc;
  doc["maxtemp"] = config.maxtemp;
  doc["mintemp"] = config.mintemp;  
  
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
  doc["trigger"] = config.trigger;

  serializeJson(doc, configweb);
  return String(configweb);
}

String getMinuteur(const Programme& minuteur ) {

  // Vérifiez si le système est prêt , protection contre une corruption de la mémoire
  if (millis() < 5000) {
        Serial.println("System not ready yet");
        return "false";
  }

  struct tm timeinfo;  // Déclaration locale
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return "false";
    }
  
  JsonDocument doc;
  doc["heure_demarrage"] = minuteur.heure_demarrage;
  doc["heure_arret"] = minuteur.heure_arret;
  doc["temperature"] = minuteur.temperature;
  doc["heure"] = timeinfo.tm_hour;
  doc["minute"] = timeinfo.tm_min;
  doc["seuil_start"] = minuteur.seuil_start;
  doc["seuil_stop"] = minuteur.seuil_stop;
  doc["seuil_temp"] = minuteur.seuil_temperature;
  doc["puissance"] = minuteur.puissance;
  String retour;
  serializeJson(doc, retour);
  return retour;
}

String getMinuteur() {

  // Vérifiez si le système est prêt , protection contre une corruption de la mémoire
  if (millis() < 5000) {
        Serial.println("System not ready yet");
        return "false";
  }

  struct tm timeinfo;  // Déclaration locale
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return "false";
    }
  
  JsonDocument doc;
  doc["heure"] = timeinfo.tm_hour;
  doc["minute"] = timeinfo.tm_min;
  String retour;
  serializeJson(doc, retour);
  return retour;
}

String getmqtt() {
  String retour;
  JsonDocument doc;
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
  return String(retour);
}

String getcomplement() {
  String retour;
  JsonDocument doc;

  doc["hdebut"] = config.hostname;
  doc["hfin"] = config.port;
  doc["tmax"] = config.maxtemp;

  serializeJson(doc, retour);
  return String(retour);
}


String readmqttsave(){
  String node_id_readmqttsave = config.say_my_name;
  auto save_command = String("Xlyric/sauvegarde/"+ node_id_readmqttsave );
  client.subscribe(save_command.c_str(),1);
  return String(
    "<html><head><meta http-equiv='refresh' content='5;url=config.html' /></head><body><h1>config restauree, retour au setup dans 5 secondes, pensez a sauvegarder sur la flash </h1></body></html>");
}

String getServermode(String Servermode) {
  if ( Servermode == "MQTT" ) {   mqtt_config.mqtt = !mqtt_config.mqtt; }
  if ( Servermode == "HA" ) {   config.HA = !config.HA; }
  if ( Servermode == "JEEDOM" ) {   config.JEEDOM = !config.JEEDOM; }
  if ( Servermode == "DOMOTICZ" ) {   config.DOMOTICZ = !config.DOMOTICZ; }
  if ( Servermode == "ONOFF" ) {   config.dimmer_on_off = !config.dimmer_on_off; }

  return String(Servermode);
}

String replaceSpacesWithHyphens(String input) {
  input.replace(' ', '-');
  return input;
}


#endif