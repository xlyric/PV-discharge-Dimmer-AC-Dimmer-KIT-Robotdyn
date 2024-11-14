/************************************************************************
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
 *  | ESP32ETH      | 39(39),     36(36),     |                         |
 *  |               | 15(15),     14(14),     | 15(15),     14(14),     |
 *  |               | 12(12),     5(5),       | 12(12),     5(5),       |
 *  |               | 4(4),       2(2),       | 4(4),       2(2),       |
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

#include <Arduino.h>

// #include "Arduino.h"


#ifdef ROBOTDYN
// Dimmer librairy
  #include <RBDdimmer.h>   /// the corrected librairy  in personal depot , the original has a bug
#endif
// Web services
#include <ESPAsyncWiFiManager.h>
#include <ESPAsyncWebServer.h>

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson v6

#include <TaskScheduler.h> // gestion des taches

// ota mise à jour sans fil
#include <ElegantOTA.h>
// Dallas 18b20
#include <OneWire.h>
#include <DallasTemperature.h>
// mqtt
#include <PubSubClient.h>
/// config
#include "langues/lang.h"
#include "config/config.h"
#include "config/enums.h"
#include "function/web.h"
#include "function/ha.h"
#include "function/littlefs.h"
#include "function/mqtt.h"
#include "function/minuteur.h"

/*
   extern "C" {
 #include "user_interface.h"
   }
 */

#ifdef ROBOTDYN
  #include "function/dimmer.h"
#endif

#ifdef  SSR
  #include "function/jotta.h"
#endif

#include "function/unified_dimmer.h"

#include "tasks/dallas.h"
#include "tasks/cooler.h"
#include "tasks/get_power.h"
#include "tasks/relais.h"
#include "tasks/ping.h"

#if defined(ESP32) || defined(ESP32ETH)
// Web services
  #include "WiFi.h"
  #include "HTTPClient.h"
  #include <driver/adc.h>
// File System
  #include <FS.h>
  #include "SPIFFS.h"
  #define LittleFS SPIFFS // Fonctionne, mais est-ce correct?
  #include <esp_system.h>
  #include <ESPmDNS.h>

#else
// Web services
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
// #include <ESPAsyncTCP.h>
  #include <ESP8266HTTPClient.h>
// File System
  #include <LittleFS.h> // NOSONAR

#endif

#ifdef ESP32ETH
  #include <ETH.h>
#endif

// taches
Task Task_dallas(8000, TASK_FOREVER, &mqttdallas);
Task Task_Cooler(15000, TASK_FOREVER, &cooler);
Task Task_GET_POWER(10000, TASK_FOREVER, &get_dimmer_child_power);
#ifdef RELAY1
Task Task_relay(20000, TASK_FOREVER, &relais_controle);
#endif
/// @brief  task de ping
Task Task_ping(120000, TASK_FOREVER, &ping);
Scheduler runner;



/***************************
 * Begin Settings
 **************************/

//***********************************
//************* Gestion du serveur WEB
//***********************************
// Create AsyncWebServer object on port 80
WiFiClient domotic_client;

WiFiClient espClient;
PubSubClient client(espClient);

bool mqttConnected = false;

DNSServer dns;
HTTPClient http;
bool shouldSaveConfig = false;
Wifi_struct wifi_config_fixe;


//***********************************
//************* Time
//***********************************
int timesync = 0;
int timesync_refresh = 120;

// *************************************

// #define USE_SERIAL  SerialUSB // Serial for boards whith USB serial port
#define USE_SERIAL  Serial

//***********************************
//************* Dallas
//***********************************
bool dallaspresent ();


////////////////////////////////////
///     AP MODE
/////////////////////////////////

String routeur="PV-ROUTER";
bool AP = false;
bool discovery_temp;


OneWire ds(ONE_WIRE_BUS);   //  (a 4.7K resistor is necessary - 5.7K work with 3.3 ans 5V power)
DallasTemperature sensors(&ds);


byte present = 0;

byte data[12];   // NOSONAR
float previous_celsius[MAX_DALLAS] = {0.00};   // NOSONAR
// byte security = 0;
int refresh = 60;
int refreshcount = 0;
int deviceCount = 0;
int devicealerte=0;
DeviceAddress addr[MAX_DALLAS];   // array of (up to) MAX_DALLAS temperature sensors NOSONAR
String devAddrNames[MAX_DALLAS];  // array of (up to) MAX_DALLAS temperature sensors NOSONAR
/***************************
 * End Settings
 **************************/

//***********************************
//************* Gestion de la configuration
//***********************************

Config config;
Mqtt mqtt_config;
System sysvar;
Programme programme;
Programme programme_relay1;
Programme programme_relay2;

String getmqtt();
void savemqtt(const char *filename, const Mqtt &mqtt_config); // NOSONAR
bool pingIP(IPAddress ip);
String stringBool(bool mybool);
String getServermode(String Servermode);
String switchstate(int state);
void tests ();

/// @brief  declaration des logs
Logs logging;/// declare logs

int childsend = 0;


/***************************
 * init Dimmer
 **************************/
#ifdef ROBOTDYN
dimmerLamp dimmer(outputPin, zerocross);         // initialise port for dimmer for ESP8266, ESP32, Arduino due boards

  #ifdef outputPin2
dimmerLamp dimmer2(outputPin2, zerocross);       // initialise port for dimmer2 for ESP8266, ESP32, Arduino due boards
  #endif

  #ifdef outputPin3
dimmerLamp dimmer3(outputPin3, zerocross);       // initialise port for dimmer3 for ESP8266, ESP32, Arduino due boards
  #endif
#endif

//// test JOTTA non random
  #ifdef SSR_ZC
  #include <Ticker.h>
Ticker timer;
SSR_BURST ssr_burst;
  #endif

gestion_puissance unified_dimmer;

//***********************************
//************* function web
//***********************************

unsigned long Timer_Cooler;

IPAddress _ip,_gw,_sn,gatewayIP;   // NOSONAR


//***********************************
//************* Setup
//***********************************

void setup() {
  Serial.begin(115200);

  // reset du bus one Wire
  // ds.reset();
  // initialisation des dallas
  sensors.begin();

  #ifdef ESP32ETH
  ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
  #endif
  logging.Set_log_init("197}11}1");

  // Correction issue full power at start
  pinMode(outputPin, OUTPUT);
  pinMode(zerocross, INPUT);
  #ifndef ESP32ETH
    #ifndef ESP32
  pinMode(D1, OUTPUT);
  digitalWrite(D1, 0);
    #endif
  #endif
  #ifdef outputPin2
  pinMode(outputPin2, OUTPUT);
  #endif
  #ifdef outputPin3
  pinMode(outputPin3, OUTPUT);
  #endif

  #ifdef POWERSUPPLY2022
  pinMode(GND_PIN, OUTPUT);  /// board bug
  digitalWrite(GND_PIN, 0);  /// board bug with pin 16

  pinMode(POS_PIN, OUTPUT);
  digitalWrite(POS_PIN, 1);
  #endif

  #if defined(ESP32) || defined(ESP32ETH)
  esp_reset_reason_t reset_reason = esp_reset_reason();
  Serial.printf("Reason for reset: %d\n", reset_reason);
  logging.log(String(Reason_for_reset), reset_reason);

  #else
  rst_info *reset_info = ESP.getResetInfoPtr();
  Serial.printf("Reason for reset: %d\n", reset_info->reason);
  logging.log(String(Reason_for_reset), reset_info->reason);

  #endif

  #ifdef RELAY1 // permet de rajouter les relais en ne modifiant que config.h, et pas seulement en STANDALONE
  pinMode(RELAY1, OUTPUT);
  digitalWrite(RELAY1, LOW);
  #endif
  #ifdef RELAY2 // permet de rajouter les relais en ne modifiant que config.h
  pinMode(RELAY2, OUTPUT);
  digitalWrite(RELAY2, LOW);
  #endif

  // cooler init
  pinMode(COOLER, OUTPUT);
  digitalWrite(COOLER, LOW);

  logging.log(
    "-- Dimmer %s (version: %s / expected FS version: %s) --",
    String(COMPILE_NAME), String(VERSION), String(FS_VERSION)
    );

  // démarrage file system
  LittleFS.begin();
  // correction d'erreur de chargement de FS
  delay(1000);
  Serial.println("Demarrage file System");
  logging.log(Start_filesystem);
  logging.log(check_fs_version(String(FS_version_is_outdated)));
#ifdef ROBOTDYN
  // configuration dimmer
    #ifdef outputPin
  dimmer.begin(NORMAL_MODE, OFF);     // dimmer initialisation: name.begin(MODE, STATE)
    #endif
    #ifdef outputPin2
  // la déclaration est en fait globale pas besoin de redéclarer et pose pb sur ESP32
  // dimmer2.begin(NORMAL_MODE, ON); // dimmer2 initialisation: name.begin(MODE, STATE)
    #endif
    #ifdef outputPin3
  // dimmer3.begin(NORMAL_MODE, ON); // dimmer3 initialisation: name.begin(MODE, STATE)
    #endif

  #ifdef POWERSUPPLY2022
  /// correct bug board
  dimmer.setState(ON);

  pinMode(GND_PIN, OUTPUT);  /// board bug
  digitalWrite(GND_PIN, 0);  /// board bug with pin 16

  pinMode(POS_PIN, OUTPUT);
  digitalWrite(POS_PIN, 1);
  #endif
#endif


  /// init de sécurité
  #ifdef ROBOTDYN
  dimmer.setState(OFF);
  #endif
  #ifdef outputPin2
  dimmer2.setState(OFF);
  #endif
  #ifdef outputPin3
  dimmer3.setState(OFF);
  #endif

  USE_SERIAL.println("Dimmer Program is starting...");

  //***********************************
  //************* Setup -  récupération du fichier de configuration
  //***********************************
  #if !defined(ESP32) && !defined(ESP32ETH)
  ESP.getResetReason();
  #endif
  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
  logging.log(Load_configuration);
  logging.log(config.loadConfiguration()); // charge la configuration

  // Load configuration file mqtt
  Serial.println(F("Loading mqtt_conf configuration..."));
  logging.log(Load_configuration_MQTT);
  logging.log(mqtt_config.loadmqtt());  // charge la configuration mqtt

  // chargement des conf de wifi
  Serial.println(F("Loading wifi configuration..."));
  loadwifiIP(wifi_conf, wifi_config_fixe);

  // chargement des conf de minuteries
  Serial.println(F("Loading minuterie"));
  programme.set_name("/dimmer");
  programme.loadProgramme();
  programme.saveProgramme();

#ifdef RELAY1
  programme_relay1.set_name("/relay1");
  programme_relay1.loadProgramme();
  programme_relay1.saveProgramme();

  programme_relay2.set_name("/relay2");
  programme_relay2.loadProgramme();
  programme_relay2.saveProgramme();
#endif
  //***********************************
  //************* Setup - Connexion Wifi
  //***********************************
  Serial.print("start Wifiautoconnect");
  logging.log(Start_Wifiautoconnect);

  // WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  // wifi_set_phy_mode(PHY_MODE_11N);



  // préparation  configuration IP fixe

  AsyncWiFiManagerParameter custom_IP_Address("server", "IP", wifi_config_fixe.static_ip, 16);
  wifiManager.addParameter(&custom_IP_Address);
  AsyncWiFiManagerParameter custom_IP_mask("mask", "mask", wifi_config_fixe.static_sn, 16);
  wifiManager.addParameter(&custom_IP_mask);
  AsyncWiFiManagerParameter custom_IP_gateway("gateway", "gateway", wifi_config_fixe.static_gw, 16);
  wifiManager.addParameter(&custom_IP_gateway);

  _ip.fromString(wifi_config_fixe.static_ip);
  _sn.fromString(wifi_config_fixe.static_sn);
  _gw.fromString(wifi_config_fixe.static_gw);

  if ( !strcmp(wifi_config_fixe.static_ip, "") == 0) {
    // set static ip

    wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
    Serial.print(String(wifi_config_fixe.static_ip));
  }

  // doit être placé avant la connexion sinon sous ESP32, l'hostname n'est pas pris en compte
  WiFi.setHostname(config.say_my_name);
  wifiManager.autoConnect(config.say_my_name);
  // Afficher le nom :
  Serial.println("say_my_name: " + String(config.say_my_name));
  DEBUG_PRINTLN("end Wifiautoconnect");
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(600);


  strcpy(wifi_config_fixe.static_ip, custom_IP_Address.getValue());
  strcpy(wifi_config_fixe.static_sn, custom_IP_mask.getValue());
  strcpy(wifi_config_fixe.static_gw, custom_IP_gateway.getValue());

  DEBUG_PRINTLN("static adress: " + String(wifi_config_fixe.static_ip) + " mask: " + String(
                  wifi_config_fixe.static_sn) + " GW: " + String(wifi_config_fixe.static_gw));

  // savewifiIP(wifi_conf, wifi_config_fixe); // NOSONAR je doute que ça soit utilisé vu que c'est en autoconnect, seul le chargement est necessaire

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }


  // WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  // wifi_set_phy_mode(PHY_MODE_11N);
  WiFi.setAutoReconnect(true);
  // WiFi.setOutputPower(20);

  /// restart si la configuration OP static est différente ip affectée suite changement ip Autoconf
  if ( !strcmp(wifi_config_fixe.static_ip, "" ) == 0 )  {
    char IP[] = "xxx.xxx.xxx.xxx";       // NOSONAR
    IPAddress ip = WiFi.localIP();
    ip.toString().toCharArray(IP, 16);
    if (!strcmp(wifi_config_fixe.static_ip,IP) == 0) {
      DEBUG_PRINTLN(wifi_config_fixe.static_ip);
      Serial.println(WiFi.localIP());

      Serial.print("Application de la nouvelle configuration Ip   ");
      config.restart = true;
    }
  }
  // Si connexion affichage info dans console
  Serial.println("");
  DEBUG_PRINTLN("Connection ok sur le reseau :  ");

  Serial.print("IP address: ");

  Serial.println(WiFi.localIP());
  gatewayIP = WiFi.gatewayIP();
  Serial.println(gatewayIP);


  #if !defined(ESP32) && !defined(ESP32ETH)
  Serial.println(ESP.getResetReason());
  #endif
  //// AP MODE
  if ( routeur.compareTo(WiFi.SSID().substring(0,9)) == 0 ) {
    AP = true;
  }

  // Initialize mDNS
  // config.say_my_name)
  if (!MDNS.begin(config.say_my_name)) {
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  // MDNS.addService("http", "tcp", 80);
  MDNS.addService("http", "tcp", 1308);

  logging.log(mDNS_Responder_Started);
  logging.log(String(config.say_my_name) + ".local");



  //***********************************
  //************* Setup - OTA
  //***********************************
  ElegantOTA.begin(&server);      // Start ElegantOTA
  ElegantOTA.setAutoReboot(true);
  //***********************************
  //************* Setup - Web pages
  //***********************************


  // chargement des url des pages
  call_pages();

  //***********************************
  //************* Setup -  demarrage du webserver et affichage de l'oled
  //***********************************
  Serial.println("start server");
  server.begin();


  //// récupération des dallas .
  Serial.println("start 18b20");

  delay(1000);
  deviceCount = sensors.getDeviceCount();

  if (deviceCount == 0 ) { // si toujours pas trouvé
    sensors.begin(); // réinit du bus one wire
    delay(1500);
    deviceCount = sensors.getDeviceCount();
  }

  logging.log(DALLAS_detected, deviceCount);

  /// recherche d'une sonde dallas
  dallaspresent();

  devices_init(); // initialisation des devices HA

  /// MQTT
  if (!AP && mqtt_config.mqtt) {
    Serial.println("Connection MQTT" );
    logging.log(Attempting_MQTT_connexion);

    /// Configuration et connexion MQTT
    async_mqtt_init();

    delay(1000);
    connectToMqtt();
    delay(1000);
    /// pour remonter un 0 sur le MQTT
    if (config.HA || config.JEEDOM) {
      device_dimmer.send(String(sysvar.puissance));
      device_dimmer_send_power.send(String(sysvar.puissance));
      device_dimmer_power.send(String(sysvar.puissance* config.charge/100));
      device_dimmer_total_power.send(String(sysvar.puissance_cumul + (sysvar.puissance * config.charge/100)));
      device_cooler.send(stringBoolMQTT(false));
      device_dimmer_starting_pow.send(String(config.startingpow));
      device_dimmer_minpow.send(String(config.minpow));
      device_dimmer_maxpow.send(String(config.maxpow));

      device_dimmer_child_mode.send(String(config.mode));
      device_dimmer_on_off.send(String(config.dimmer_on_off));

      #ifdef RELAY1
      int relaystate = digitalRead(RELAY1);
      device_relay1.send(String(relaystate));
      #endif
      #ifdef RELAY2
      relaystate = digitalRead(RELAY2);
      device_relay2.send(String(relaystate));
      #endif
      HA_discover();
    }
  }


  #ifdef  SSR
    #ifdef OLDSSR
  analogWriteFreq(GRIDFREQ);
  analogWriteRange(100);
  analogWrite(JOTTA, 0);
    #elif  defined(SSR_ZC)
  pinMode(JOTTA, OUTPUT);
  unified_dimmer.set_power(0);
  timer.attach_ms(10, SSR_run);     // Attachez la fonction task() au temporisateur pour qu'elle s'exécute toutes les 1000 ms
    #else
  init_jotta();
  timer_init();
    #endif
  #endif


  /// init du NTP
  ntpinit();

  /// init des tasks
  runner.init();
  runner.addTask(Task_dallas);
  Task_dallas.enable();

  runner.addTask(Task_Cooler);
  Task_Cooler.enable();

  runner.addTask(Task_GET_POWER);
  Task_GET_POWER.enable();

  runner.addTask(Task_ping);
  Task_ping.enable();

  DEBUG_PRINTLN(ESP.getFreeHeap());

  /// affichage de l'heure  GMT +1 dans la log
  logging.log(End_Start);

  delay(1000);
}


bool alerte=false;

/////////////////////
/// LOOP
/////////////////////
void loop() {
  client.loop();

  #if !defined(ESP32) && !defined(ESP32ETH)
  /// update mdns
  MDNS.update();
  #endif

  /// connexion MQTT dans les cas de conf mqtt et perte de connexion
  if (!client.connected() ) {
    mqttConnected = false;
  }
  if (!mqttConnected && !AP && mqtt_config.mqtt) {
    connect_and_subscribe();
  }

  runner.execute(); // gestion des taches

  /// limitation de la taille de la chaine de log
  logging.clean_log_init();

  /// detection de la perte de wifi
  if (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("NO WIFI - Restarting Dimmer");
    config.restart = true;
  }

  ///////////////// gestion des activité minuteur
  //// Dimmer
  if (programme.run) {
    //  minuteur en cours
    if (programme.stop_progr()) {
      // Robotdyn dimmer
      logging.log(Stop_minuteur);
      unified_dimmer.set_power(0);       // necessaire pour les autres modes
      unified_dimmer.dimmer_off();

      DEBUG_PRINTLN("programme.run");
      sysvar.puissance=0;
      Serial.print(Stop_minuteur);
      Mqtt_send_DOMOTICZ(String(config.IDX), String (sysvar.puissance * config.charge/100) );   // remonté MQTT de la commande réelle
      if (config.HA) {
        int instant_power = unified_dimmer.get_power();
        device_dimmer_on_off.send(String(config.dimmer_on_off));
        device_dimmer.send(String(instant_power));
        device_dimmer_send_power.send(String(instant_power));
        device_dimmer_power.send(String(instant_power * config.charge/100));
        device_dimmer_total_power.send(String(sysvar.puissance_cumul + (sysvar.puissance * config.charge/100)));
      }
      // réinint de la sécurité température
      sysvar.security = 0;
    }
  }
  else {
    // minuteur à l'arret
    if (programme.start_progr()) {
      // definition de la puissance à appliquer
      if ( programme.puissance > config.maxpow ) {     sysvar.puissance=config.maxpow; }
      else { sysvar.puissance = programme.puissance; }

      //// robotdyn dimmer
      logging.log(Start_minuteur);

      unified_dimmer.set_power(sysvar.puissance);
      delay (50);

      Serial.print(Start_minuteur);
      Mqtt_send_DOMOTICZ(String(config.IDX), String (sysvar.puissance * config.charge/100) ); // remonté MQTT de la commande réelle
      if (config.HA) {
        int instant_power = unified_dimmer.get_power();
        device_dimmer_on_off.send(String(config.dimmer_on_off));
        device_dimmer.send(String(instant_power));
        device_dimmer_send_power.send(String(instant_power));
        device_dimmer_power.send(String(instant_power * config.charge/100));
        device_dimmer_total_power.send(String(sysvar.puissance_cumul + (sysvar.puissance * config.charge/100)));
      }
    }
  }


#ifdef RELAY1
  //// relay 1
  if (programme_relay1.run) {
    if (programme_relay1.stop_progr()) {
      logging.log(Stop_minuteur_relay1);
      digitalWrite(RELAY1, LOW);
      device_relay1.send(String(0));
    }
  }
  else {
    if (programme_relay1.start_progr()) {
      logging.log(Start_minuteur_relay1);
      digitalWrite(RELAY1, HIGH);
      device_relay1.send(String(1));
    }
  }

  if (programme_relay2.run) {
    if (programme_relay2.stop_progr()) {
      logging.log(Stop_minuteur_relay2);
      digitalWrite(RELAY2, LOW);
      device_relay2.send(String(0));
    }
  }
  else {
    if (programme_relay2.start_progr()) {
      logging.log(Start_minuteur_relay2);
      digitalWrite(RELAY2, HIGH);
      device_relay2.send(String(1));
    }
  }
#endif

  ///////////////// commande de restart /////////
  if (config.restart) {
    delay(5000);
    Serial.print("Restarting Dimmer");
    ESP.restart();
  }

  //// si la sécurité température est active on coupe le dimmer
  if ( sysvar.celsius[sysvar.dallas_maitre] > ( config.maxtemp + 2) && (!alerte) ) {
    logging.Set_alerte_web("Surchauffe ("+String(sysvar.celsius[sysvar.dallas_maitre])+"°C)");
    if (mqtt_config.mqtt)
      Mqtt_send_DOMOTICZ(String(config.IDXAlarme), "Surchauffe ("+String(
                           sysvar.celsius[sysvar.dallas_maitre])+"°C)", "Alerte");
    if (config.HA)
      device_dimmer_alarm.send("Surchauffe ("+String(sysvar.celsius[sysvar.dallas_maitre])+"°C)");
    alerte=true;
    unified_dimmer.dimmer_off();
  }

  if ( sysvar.security == 1 ) {
    if (!alerte) {
      Serial.println("Alert Temp");
      logging.log(Alert_Temp);

      if (!AP && mqtt_config.mqtt ) {
        logging.Set_alerte_web("Surchauffe ("+String(sysvar.celsius[sysvar.dallas_maitre])+"°C)");
        if (mqtt_config.mqtt)
          Mqtt_send_DOMOTICZ(String(config.IDXAlarme), "Surchauffe ("+String(
                               sysvar.celsius[sysvar.dallas_maitre])+"°C)", "Alerte");
        if (config.HA)
          device_dimmer_alarm.send("Surchauffe ("+String(sysvar.celsius[sysvar.dallas_maitre])+"°C)");
      }
      alerte=true;
    }
    //// Trigger de sécurité température
    if ( sysvar.celsius[sysvar.dallas_maitre] <= (config.maxtemp - (config.maxtemp*config.trigger/100)) ) {
      sysvar.security = 0;
      logging.Set_alerte_web("");
      if (mqtt_config.mqtt) Mqtt_send_DOMOTICZ(String(config.IDXAlarme), check_fs_version("RAS"), "Alerte");
      if (config.HA) device_dimmer_alarm.send(check_fs_version("RAS"));
      sysvar.change = 1;
    }
    else {
      unified_dimmer.dimmer_off();
    }
  }
  else {
    alerte=false;
  }

  ////////////////// controle de la puissance /////////////////

  if ( sysvar.change == 1  && programme.run == false ) {   /// si changement et pas de minuteur en cours

    if (config.dimmer_on_off == 0) {
      unified_dimmer.dimmer_off();
    }

    /// si on dépasse la puissance mini demandée
    DEBUG_PRINTLN(("%d------------------",__LINE__));
    DEBUG_PRINTLN(sysvar.puissance);

    if (sysvar.puissance_cumul != 0) {
      if ( strcmp(config.child,"") != 0 && strcmp(config.child,"none") != 0 && strcmp(config.mode,"off") == 0 ) {
        child_communication(0,false);
        // Du coup je force sysvar.puissance_cumul à 0 puisque Task_GET_POWER ne renverra plus rien désormais
        // ça évitera de rentrer dans cette boucle à l'infini en bombardant le dimmer d'ordres à 0 pour rien
        sysvar.puissance_cumul = 0;
      }
    }
    if (sysvar.puissance > config.minpow && sysvar.puissance != 0 && sysvar.security == 0)
    {
      DEBUG_PRINTLN(("%d------------------",__LINE__));
      /// si au dessus de la consigne max configurée alors config.maxpow.
      if ( sysvar.puissance > config.maxpow )   // || sysvar.puissance_cumul > sysvar.puissancemax )
      {
        if (config.dimmer_on_off == 1) {
          unified_dimmer.set_power(config.maxpow);
          DEBUG_PRINTLN(("%d------------------",__LINE__));
        }
        /// si on a une carte fille et qu'elle n'est pas configurée sur off, on envoie la commande
        if ( strcmp(config.child,"") != 0 && strcmp(config.child,"none") != 0 && strcmp(config.mode,"off") != 0 ) {
          if ( strcmp(config.mode,"delester") == 0 ) {
            child_communication(int((sysvar.puissance-config.maxpow)),true );      // si mode délest, envoi du surplus
          }
          if ( strcmp(config.mode,"equal") == 0) {
            child_communication(sysvar.puissance,true);       // si mode equal envoie de la commande vers la carte fille
          }
        }
        DEBUG_PRINTLN(("%d------------------",__LINE__));
        DEBUG_PRINTLN(sysvar.puissance);
      }
      /// fonctionnement normal
      else
      {
        if (config.dimmer_on_off == 1) {
          unified_dimmer.set_power(sysvar.puissance);
        }

        if ( strcmp(config.child,"") != 0 && strcmp(config.child,"none") != 0 ) {
          // si mode equal envoie de la commande vers la carte fille
          if ( strcmp(config.mode,"equal") == 0) {
            child_communication(int(sysvar.puissance),true);
          }
          // si mode délest envoie d'une commande à 0
          if ( strcmp(config.mode,"delester") == 0 && sysvar.puissance <= config.maxpow) {
            child_communication(0,false);
            logging.log(Child_at_zero);
          }
          // si sysvar.puissance passe subitement au dessus de config.maxpow
          if ( strcmp(config.mode,"delester") == 0 && sysvar.puissance > config.maxpow) {
            child_communication(int((sysvar.puissance-config.maxpow)),true );
          }
          DEBUG_PRINTLN(("%d  -----------------",__LINE__));
          DEBUG_PRINTLN(sysvar.puissance);
        }
      }
      DEBUG_PRINTLN(("%d------------------",__LINE__));
      DEBUG_PRINTLN(sysvar.puissance);

      /// si on est en mode MQTT on remonte les valeurs vers HA et MQTT
      if (!AP && mqtt_config.mqtt) {
        if (config.dimmer_on_off == 0) {
          Mqtt_send_DOMOTICZ(String(config.IDX), String (sysvar.puissance * config.charge/100) );  // remonté MQTT de la commande 0
          device_dimmer.send("0");  // remonté MQTT HA de la commande 0
          device_dimmer_send_power.send("0");
          device_dimmer_power.send("0");

        }
        else if ( sysvar.puissance > config.maxpow ) {
          Mqtt_send_DOMOTICZ(String(config.IDX), String (sysvar.puissance * config.charge/100) );  // remonté MQTT de la commande max
          if (config.HA) {
            device_dimmer.send(String(config.maxpow));
            /// Modif RV - 20240219
            /// Oubli d'envoie "device_dimmer_power.send" + correction de "device_dimmer_total_power.send"
            device_dimmer_power.send(String(config.maxpow * config.charge/100));
            device_dimmer_total_power.send(String(sysvar.puissance_cumul + (config.maxpow * config.charge/100)));
          }                                                                                                          // remonté MQTT HA de la commande max
        }
        else {
          Mqtt_send_DOMOTICZ(String(config.IDX), String (sysvar.puissance * config.charge/100)); // remonté MQTT de la commande réelle
          if (config.HA) {
            int instant_power = unified_dimmer.get_power();
            device_dimmer.send(String(instant_power));
            device_dimmer_power.send(String(instant_power * config.charge/100));
            device_dimmer_total_power.send(String(sysvar.puissance_cumul + (instant_power * config.charge/100)));
          }   // remonté MQTT HA de la commande réelle
        }
      }
    }
    /// si la sécurité est active on déleste
    else if ( sysvar.puissance != 0 && sysvar.security == 1)
    {

      if ( strcmp(config.child,"") != 0 && strcmp(config.child,"none") != 0  && strcmp(config.mode,"off") != 0) {
        if (sysvar.puissance > 200 ) {sysvar.puissance = 200;}

        // si mode délest, envoi du surplus
        if ( strcmp(config.mode,"delester") == 0 ) { child_communication(int(sysvar.puissance),true); childsend = 0; }
        // si mode equal envoie de la commande vers la carte fille
        if ( strcmp(config.mode,"equal") == 0) { child_communication(sysvar.puissance,true); childsend = 0; }
      }
    }
    //// si la commande est trop faible on coupe tout partout
    else if ( sysvar.puissance <= config.minpow ) {
      DEBUG_PRINTLN("commande est trop faible");
      unified_dimmer.set_power(0);   // necessaire pour les autres modes
      unified_dimmer.dimmer_off();
      /// et sur les sous routeur
      if ( strcmp(config.child,"") != 0 && strcmp(config.child,"none") != 0 ) {
        if ( strcmp(config.mode,"delester") == 0 ) { child_communication(0,false); }     // si mode délest, envoi du surplus
        if ( strcmp(config.mode,"equal") == 0) { child_communication(0,false); }      // si mode equal envoie de la commande vers la carte fille
        if ( strcmp(config.mode,"off") != 0) {
          if (childsend>2) {
            child_communication(0,false);
            childsend++;
          }
        }

        if ( mqtt_config.mqtt ) {
          Mqtt_send_DOMOTICZ(String(config.IDX), String (sysvar.puissance * config.charge/100) );
        }
        if ( config.HA ) {
          device_dimmer.send("0");
          device_dimmer_power.send("0");
        }
      }

      if (!AP && mqtt_config.Mqtt::mqtt) {
        int instant_power = unified_dimmer.get_power();
        Mqtt_send_DOMOTICZ(String(config.IDX), String (sysvar.puissance * config.charge/100) );    // correction 19/04
        device_dimmer.send(String(instant_power));
        device_dimmer_send_power.send(String(instant_power));
        device_dimmer_power.send(String(instant_power * config.charge/100));
        device_dimmer_total_power.send(String(sysvar.puissance_cumul + (instant_power*config.charge/100) ));
      }
    }

    sysvar.change = 0; /// déplacé ici à la fin
  }

  //***********************************
  //************* LOOP - Activation de la sécurité --> doublon partiel avec la fonction sécurité ?
  //***********************************
  if ( sysvar.celsius[sysvar.dallas_maitre] >= config.maxtemp && sysvar.security == 0 ) {
    sysvar.security = 1;
    unified_dimmer.set_power(0); // necessaire pour les autres modes
    unified_dimmer.dimmer_off();
    /// pour être sur que la dernière consigne envoyé soit au moins égale au max.temp
    float temp = sysvar.celsius[sysvar.dallas_maitre] + 0.2;
    Mqtt_send_DOMOTICZ(String(config.IDXTemp), String(temp),"Temperature"); /// remonté MQTT de la température
    if ( config.HA ) {
      device_temp[sysvar.dallas_maitre].send(String(temp));
      device_temp_master.send(String(temp));
      device_dimmer_power.send(String(0));
      device_dimmer_total_power.send(String(sysvar.puissance_cumul));
    }        /// si HA remonté MQTT HA de la température
    logging.Set_alerte_web("Surchauffe ("+String(temp)+"°C)");
    if (mqtt_config.mqtt) Mqtt_send_DOMOTICZ(String(config.IDXAlarme), "Surchauffe ("+String(temp)+"°C)", "Alerte");
    if (config.HA) device_dimmer_alarm.send("Surchauffe ("+String(temp)+"°C)");
  }

  //// protection contre la perte de la sonde dallas
  if (strlen(config.SubscribeTEMP) == 0 ) {
    restart_dallas();
  }
  delay(100); // 24/01/2023 changement 500 à 100ms pour plus de réactivité
}

///////////////
//// fin de loop
//////////////

//***********************************
//************* Test de la présence d'une 18b20
//***********************************

bool dallaspresent () {

  /// alerte d'une deection de dallas passée non trouvée
  if (deviceCount == 0 && ( strcmp("null", config.DALLAS) != 0 || strcmp("none", config.DALLAS) != 0 )) {
    /// remonter l'alerte une fois toute les 10 secondes
    if (devicealerte == 0) {
      logging.log(Alerte_Dallas_not_found);
      logging.Set_alerte_web("Sonde température (Dallas) non-détectée");
      if (mqtt_config.mqtt)
        Mqtt_send_DOMOTICZ(
          String(config.IDXAlarme),
          "Sonde température (Dallas) non-détectée",
          "Alerte"
          );
      if (config.HA) device_dimmer_alarm.send("Sonde température (Dallas) non-détectée");
      devicealerte++;
    }
    else {
      devicealerte++;
      if (devicealerte > 10) {
        devicealerte = 0;
      }
    }
    return false;
  }

  logging.Set_alerte_web("");
  if (mqtt_config.mqtt) Mqtt_send_DOMOTICZ(String(config.IDXAlarme), check_fs_version("RAS"), "Alerte");
  if (config.HA) device_dimmer_alarm.send(check_fs_version("RAS"));

  //// recherche des adresses des sondes

  for (int i = 0; i < deviceCount; i++) {
    // int attempts = 0;
    // bool sensorFound = false;
    /// cas pour les sondes les plus lentes
    /*   while (!sensorFound) {
         if (!ds.search(addr[i])) {
           delay(200); // try again after a short delay
           attempts++;
           if (attempts > 5) { // timeout: 5 attempts * 200ms each
             break;
           }
         } else {
           sensorFound = true;
         }
       }

       /// si pas de sonde trouvée
       if (!sensorFound) {
         logging.log("Unable to find temperature sensors address");
         return false;
       }*/

    if (!sensors.getAddress(addr[i], i)) Serial.println("Unable to find address for Device 1");
    else {
      sensors.setResolution(addr[i], TEMPERATURE_PRECISION);
    }
  }

  for (int a = 0; a < deviceCount; a++) {
    String address = "";
    Serial.print("ROM =");
    for (uint8_t i = 0; i < 8; i++) {
      if (addr[a][i] < 0x10) address += "0";
      address += String(addr[a][i], HEX);
      Serial.write(' ');
      Serial.print(addr[a][i], HEX);
    }
    devAddrNames[a] = address;
    Serial.println();
    logging.log(Dallas_sensor_found, a, address.c_str());

    if (strcmp(address.c_str(), config.DALLAS) == 0) {
      sysvar.dallas_maitre = a;
      logging.log(Dallas_sensor_is_main, address.c_str());
    }
    // détection de la 1ere présence d'une dallas
    else if (strcmp("none", config.DALLAS) == 0 || strcmp("null", config.DALLAS) == 0 ) {
      sysvar.dallas_maitre = a;
      // sauvegarde de l'adresse de la sonde maitre
      strcpy(config.DALLAS, address.c_str());
      config.saveConfiguration();
      logging.log(Dallas_sensor_is_now_main, address);
    }


    present = 1;

    delay(250);
    ds.reset();
    ds.select(addr[a]);

    ds.write(0x44, 1);        // start conversion, with parasite power on at the end

    delay(1000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.

    present = ds.reset();    ///  byte 0 > 1 si present
    delay(1000);

    ds.select(addr[a]);
    ds.write(0xBE);         // Read Scratchpad


  }
  return true;
}


String stringBool(bool myBool) {
  return myBool ? "true" : "false";
}


void tests () {
/// test de debug
#ifdef ROBOTDYN
  USE_SERIAL.println("--- Toggle dimmer example start ---");
  dimmer.setState(ON); // state: dimmer1.setState(ON/OFF);
  dimmer.setState(ON);
  dimmer3.setState(ON);
  dimmer2.setState(ON);
  dimmer2.setPower(100);
  dimmer3.setPower(100);
  dimmer.setPower(100);
  delay(5000);
  dimmer.setPower(0);
  dimmer2.setPower(0);
  dimmer3.setPower(0);
  delay(5000);
  USE_SERIAL.println("--- Toggle dimmer example end ---");
#endif
}

