#ifndef TASK_DALLAS
#define TASK_DALLAS

#ifdef STANDALONE
// #include <Pinger.h>
// Pinger pinger;
#endif

extern PubSubClient client;
extern DallasTemperature sensors;
extern bool AP; // mode point d'accès
extern Mqtt mqtt_config; // configuration mqtt
extern byte present; // capteur dallas présent ou non
extern String logs; // logs
// extern byte security; // sécurité
extern DeviceAddress addr[MAX_DALLAS];  // NOSONAR
extern float previous_celsius[MAX_DALLAS]; // température précédente // NOSONAR
extern IPAddress gatewayIP;
extern HA devicetemp[MAX_DALLAS]; // NOSONAR
extern int deviceCount; // nombre de sonde(s) dallas détectée(s)
int dallas_error[MAX_DALLAS] = {0}; // compteur d'erreur dallas // NOSONAR
int gw_error = 0;   // compteur d'erreur gateway
int dallas_wait_log; // NOSONAR

float CheckTemperature(String label, byte deviceAddress[12]); // NOSONAR
void restart_dallas();
bool dallaspresent ();
int timer_dallas = 500; // timer pour la dallas

/// @brief / task executé toute les n secondes pour publier la température ( voir déclaration task dans main )
void mqttdallas() {
  if ( present == 1 ) {
    sensors.requestTemperatures();
    // delai plu utile vu que demande de relevé fait il y a ~15 sec
    delay(timer_dallas);
    for (int a = 0; a < deviceCount; a++) {
      sysvar.celsius[a]=CheckTemperature("temp_" + devAddrNames[a],addr[a]);
      // gestion des erreurs DS18B20
      if ( (sysvar.celsius[a] == DEVICE_DISCONNECTED_C) || (sysvar.celsius[a] == -255.00) ||
           (sysvar.celsius[a] > 200.00) ) {
        sysvar.celsius[a]=previous_celsius[a];
        dallas_error[a]++;  // incrémente le compteur d'erreur

        logging.Set_log_init("Dallas " + String(a) + " : échec "+ String(dallas_error[a]) + "\r\n",true);
        if ( timer_dallas < DALALS_TIMEOUT )  { timer_dallas = timer_dallas + 100;  } // on augmente le timer pour la prochaine lecture

      }
      else {
        sysvar.celsius[a] = (roundf(sysvar.celsius[a] * 10) / 10 ) + 0.1; // pour les valeurs min
        dallas_error[a] = 0; // remise à zéro du compteur d'erreur
      }
    }
    if (!AP && mqtt_config.mqtt) {
      if ( sysvar.celsius[sysvar.dallas_maitre] != previous_celsius[sysvar.dallas_maitre]  ||
           sysvar.celsius[sysvar.dallas_maitre] != 0.99) {
        Mqtt_send_DOMOTICZ(String(config.IDXTemp), String(sysvar.celsius[sysvar.dallas_maitre]),"Temperature");
      }

      if (!discovery_temp) {
        discovery_temp = true;
        device_dimmer_alarm.HA_discovery();
        for (int i = 0; i < deviceCount; i++) {
          device_temp[i].HA_discovery();
        }
        device_temp_master.HA_discovery();
        device_dimmer_maxtemp.HA_discovery();
        device_dimmer_alarm.send(check_fs_version("RAS"));
        device_dimmer_maxtemp.send(String(config.maxtemp));
        device_dimmer_reset_alarm.HA_discovery();
      }

      // uniformisation des valeurs de température ( for en valeur I pour retrouver plus facilement)
      dallas_wait_log++;
      for (int i = 0; i < deviceCount; i++) {

        if ( sysvar.celsius[i] != previous_celsius[i] || sysvar.celsius[i] != 0.99) {
          device_temp[i].send(String(sysvar.celsius[i]));
          previous_celsius[i]=sysvar.celsius[i];
          if ( dallas_wait_log > 5 ) { /// limitation de l'affichage des logs de température
            logging.log("Dallas %d : %s°C", i, String(sysvar.celsius[i]));
          }
        }
      }
      if ( dallas_wait_log > 5 ) { dallas_wait_log = 0; }
      device_temp_master.send(String(sysvar.celsius[sysvar.dallas_maitre]));
    }
    // on demande la température suivante pour le prochain cycle

  }
  // détection sécurité température
  if  ( sysvar.celsius[sysvar.dallas_maitre] >= config.maxtemp ) {
    // coupure du dimmer
    DEBUG_PRINTLN("détection sécurité température");

    unified_dimmer.dimmer_off();

    if ( strcmp(config.child,"") != 0 && strcmp(config.mode,"off") != 0) {
      // si ça n'est pas le cas, on ne fait rien ... c'est bien parfois de ne rien faire
    }
    else {
      sysvar.puissance=0;
      unified_dimmer.dimmer_off();
    }

    if ( mqtt_config.mqtt ) {
      Mqtt_send_DOMOTICZ(String(config.IDX), "0");
    }
    if ( config.HA ) {
      device_dimmer.send("0");
      device_dimmer_power.send("0");

    }
  }

  previous_celsius[sysvar.dallas_maitre]=sysvar.celsius[sysvar.dallas_maitre];

  // si trop d'erreur dallas, on remonte en mqtt
  for (int a = 0; a < deviceCount; a++) {
    if ( dallas_error[a] > 5 ) {
      DEBUG_PRINTLN("détection perte sonde dallas");

      Mqtt_send_DOMOTICZ(String(config.IDXAlarme), String(Dallas_lost),"Dallas perdue");
      logging.log(Dallas_lost);
      dallas_error[a] = 0; // remise à zéro du compteur d'erreur
      /// mise en sécurité
      sysvar.celsius[a] = 99.9;

      previous_celsius[a]=sysvar.celsius[a];
      if (a == sysvar.dallas_maitre) {
        String temp_topic = "Xlyric/" + String(config.say_my_name) + "/dallas";

        String message = logging.get_current_time() + " - " + Dallas_lost;
        client.publish((temp_topic+"dallas").c_str(), String(message).c_str(),true);

        unified_dimmer.dimmer_off(); /// mise en sécurité de l'ensemble
      }
    }
  }


#ifdef qsdfsqdsfqs
  if ( pinger.Ping(WiFi.gatewayIP())) {
    gw_error = 0; // remise à zéro du compteur d'erreur
  }
  else {
    gw_error++;  // incrémente le compteur d'erreur
  }

  /// si GW perdu, reboot de l'ESP après 2 minutes
  if ( gw_error > 8 ) {
    DEBUG_PRINTLN("détection perte gateway");
    config.restart = true;
  }
#endif

}

//***********************************
//************* récupération d'une température du 18b20
//***********************************

float CheckTemperature(String label, byte deviceAddress[12]){ // NOSONAR
  float tempC = sensors.getTempC(deviceAddress);
  if ( (tempC == DEVICE_DISCONNECTED_C) || (tempC == -255.00) ) {
    delay(500);
    // cas d'une sonde trop longue à préparer les valeurs
    // attente de 187ms ( temps de réponse de la sonde )
    tempC = sensors.getTempC(deviceAddress);
  }
  return tempC;
}

// fonction pour relancer une détection de la dallas en cas de perte ou de non détection
void restart_dallas() {
  if (deviceCount == 0 ) {
    sensors.begin();
    delay(DALALS_TIMEOUT);
    deviceCount = sensors.getDeviceCount();
    if ( deviceCount > 0 )  {
      present = 1;
      logging.log(DALLAS_detected, deviceCount);
      devices_init(); // initialisation des devices HA
    }

    if (!dallaspresent()) {
      delay(3000);
    }
  }
}



#endif