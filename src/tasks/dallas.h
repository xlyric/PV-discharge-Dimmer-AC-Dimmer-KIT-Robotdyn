#ifndef TASK_DALLAS
#define TASK_DALLAS

#ifdef STANDALONE
#include <Pinger.h>
Pinger pinger;
#endif

extern AsyncMqttClient  client; 
extern DallasTemperature sensors;
extern bool AP; // mode point d'accès
extern Mqtt mqtt_config; // configuration mqtt
extern byte present; // capteur dallas présent ou non 
extern String logs; // logs
extern byte security; // sécurité
extern DeviceAddress addr[MAX_DALLAS];  // NOSONAR
extern float previous_celsius[MAX_DALLAS]; // température précédente //NOSONAR
extern IPAddress gatewayIP;
extern HA devicetemp[MAX_DALLAS]; // NOSONAR
extern int deviceCount; // nombre de sonde(s) dallas détectée(s)
int dallas_error[MAX_DALLAS] = {0}; // compteur d'erreur dallas // NOSONAR
int gw_error = 0;   // compteur d'erreur gateway

float CheckTemperature(String label, byte deviceAddress[12]); // NOSONAR
void restart_dallas();
void dallaspresent ();

/// @brief / task executé toute les n secondes pour publier la température ( voir déclaration task dans main )
void mqttdallas() { 
        if ( present == 1 ) {
    sensors.requestTemperatures();
    delay(400);
    for (int a = 0; a < deviceCount; a++) {
      sysvar.celsius[a]=CheckTemperature("temp_" + devAddrNames[a],addr[a]);
      //gestion des erreurs DS18B20
      if ( (sysvar.celsius[a] == -127.00) || (sysvar.celsius[a] == -255.00) || (sysvar.celsius[a] > 200.00) ) {
        sysvar.celsius[a]=previous_celsius[a];
        dallas_error[a] ++; // incrémente le compteur d'erreur
        logging.Set_log_init("Dallas" + String(a) + " : échec "+ String(dallas_error[a]) + "\r\n",true);
          }
          else { 
        sysvar.celsius[a] = (roundf(sysvar.celsius[a] * 10) / 10 ) + 0.1; // pour les valeurs min
        dallas_error[a] = 0; // remise à zéro du compteur d'erreur
      }   
    }
    if (!AP && mqtt_config.mqtt) {
      if ( sysvar.celsius[sysvar.dallas_maitre] != previous_celsius[sysvar.dallas_maitre]  || sysvar.celsius[sysvar.dallas_maitre] != 0.99) {
        Mqtt_send_DOMOTICZ(String(config.IDXTemp), String(sysvar.celsius[sysvar.dallas_maitre]),"Temperature");
      }

      if (!discovery_temp) {
        discovery_temp = true;
        device_dimmer_alarm_temp.HA_discovery();
        for (int i = 0; i < deviceCount; i++) {
          device_temp[i].HA_discovery();
        }
        device_temp_master.HA_discovery();
        device_dimmer_maxtemp.HA_discovery();
        device_dimmer_alarm_temp.send(stringBoolMQTT(sysvar.security));
        device_dimmer_maxtemp.send(String(config.maxtemp)); 
        device_dimmer_alarm_temp_clear.HA_discovery();
      }

/// uniformisation des valeurs de température ( for en valeur I pour retrouver plus facilement)
      for (int i = 0; i < deviceCount; i++) {
        if ( sysvar.celsius[i] != previous_celsius[i] || sysvar.celsius[i] != 0.99) {
          device_temp[i].send(String(sysvar.celsius[i]));
          previous_celsius[i]=sysvar.celsius[i];
        }
      }
      device_temp_master.send(String(sysvar.celsius[sysvar.dallas_maitre]));
    }          
    
         } 
    //// détection sécurité température
  if  ( sysvar.celsius[sysvar.dallas_maitre] >= config.maxtemp ) {
        // coupure du dimmer
        DEBUG_PRINTLN("détection sécurité température");

      
        unified_dimmer.dimmer_off();
        
      
      if ( strcmp(config.child,"") != 0 && strcmp(config.mode,"off") != 0){
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

    Mqtt_send_DOMOTICZ(String(config.IDXAlarme), String("Dallas perdue"),"Dallas perdue");
    logging.Set_log_init("Dallas perdue !!!\r\n",true);
      dallas_error[a] = 0; // remise à zéro du compteur d'erreur
    ///mise en sécurité
      sysvar.celsius[a] = 99.9; 

      previous_celsius[a]=sysvar.celsius[a];
      if (a == sysvar.dallas_maitre) {
       String temp_topic = "Xlyric/" + String(config.say_my_name) + "/dallas" ;

       String message = String(logging.loguptime()) + "Dallas maitre perdue";
       client.publish((temp_topic+"dallas").c_str(),1,true, String(message).c_str());

    unified_dimmer.dimmer_off();  /// mise en sécurité de l'ensemble

      }
      }
    }


#ifdef STANDALONE
  if ( pinger.Ping(WiFi.gatewayIP())) {
    gw_error = 0; // remise à zéro du compteur d'erreur
  }
  else {
    gw_error ++; // incrémente le compteur d'erreur
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

  if ( (tempC == -127.00) || (tempC == -255.00) ) {
    delay(250);
    //// cas d'une sonde trop longue à préparer les valeurs 
     /// attente de 187ms ( temps de réponse de la sonde )
    tempC = sensors.getTempC(deviceAddress);
  }  
  return tempC; 
}

/// fonction pour relancer une détection de la dallas en cas de perte ou de non détection
void restart_dallas() {
  if (deviceCount == 0 ) {
    sensors.begin();
    deviceCount = sensors.getDeviceCount();
    if ( deviceCount > 0 )  {
      present = 1;
      logging.Set_log_init(String(deviceCount)); 
      logging.Set_log_init(" DALLAS detected\r\n");
    }

    dallaspresent();
  }
}



#endif