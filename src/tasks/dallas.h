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
extern byte addr[8]; 
extern float previous_celsius; // température précédente
extern IPAddress gatewayIP;


int dallas_error = 0; // compteur d'erreur dallas
int gw_error = 0;   // compteur d'erreur gateway

float CheckTemperature(String label, byte deviceAddress[12]);
void dallaspresent ();

/// @brief / task executé toute les n secondes pour publier la température ( voir déclaration task dans main )
void mqttdallas() {
        if ( present == 1 ) {
          sysvar.celsius=CheckTemperature("Inside : ", addr); 
        
          // arrondi à 1 décimale 
          if ( (sysvar.celsius == -127.00) || (sysvar.celsius == -255.00) ) {
          sysvar.celsius=previous_celsius;
          dallas_error ++; // incrémente le compteur d'erreur
          }
          else { 
            sysvar.celsius = (roundf(sysvar.celsius * 10) / 10 ) + 0.1; // pour les valeurs min
            dallas_error = 0; // remise à zéro du compteur d'erreur
          }
          if (mqtt_config.mqtt)  { 
            if ( sysvar.celsius != previous_celsius ) {
              // envoie des infos en mqtt dans ce cas
              mqtt(String(config.IDXTemp), String(sysvar.celsius),"Temperature");
              if ( mqtt_config.HA ) { device_temp.send(String(sysvar.celsius)); }
              logs += "Dallas temp : "+ String(sysvar.celsius) +"\r\n";
            }
          }
         } 
    //// détection sécurité température
    if  ( sysvar.celsius >= config.maxtemp ) {
        // coupure du dimmer
        DEBUG_PRINTLN("détection sécurité température");
        sysvar.puissance=0;
        
      if ( mqtt_config.mqtt ) {
        mqtt(String(config.IDX), "0","pourcent");
      }
      if ( mqtt_config.HA ) { 
        device_dimmer.send("0"); 
        device_dimmer_power.send("0");
      }
    }
  
  previous_celsius=sysvar.celsius;

  // si trop d'erreur dallas, on remonte en mqtt
  if ( dallas_error > 8 ) {
    DEBUG_PRINTLN("détection perte sonde dallas");
    mqtt(String(config.IDXAlarme), String("Dallas perdue"),"Dallas perdue");
    dallas_error = 0; // remise à zéro du compteur d'erreur
  }

#ifdef STANDALONE
  if ( pinger.Ping(WiFi.gatewayIP())) {
    //Serial.println("Ping OK");
    //ESP.restart();
    gw_error = 0; // remise à zéro du compteur d'erreur
  }
  else {
    //Serial.println("perte de ping");
    //ESP.restart();
    gw_error ++; // incrémente le compteur d'erreur
  }

/// si GW perdu, reboot de l'ESP après 2 minutes
  if ( gw_error > 8 ) {
    DEBUG_PRINTLN("détection perte gateway");
    ESP.restart();
  }
#endif

}
 
    //***********************************
    //************* récupération d'une température du 18b20
    //***********************************

float CheckTemperature(String label, byte deviceAddress[12]){

  sensors.requestTemperatures();
  delay(200);
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print(label);
  if ( (tempC == -127.00) || (tempC == -255.00) ) {
    delay(250);
    //// cas d'une sonde trop longue à préparer les valeurs 
     /// attente de 187ms ( temps de réponse de la sonde )
    tempC = sensors.getTempC(deviceAddress);
  }  
  return (tempC); 
}


#endif