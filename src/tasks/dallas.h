#ifndef TASK_DALLAS
#define TASK_DALLAS

extern AsyncMqttClient  client;
extern DallasTemperature sensors;
extern bool AP; // mode point d'accès
extern Mqtt mqtt_config; // configuration mqtt
extern byte present; // capteur dallas présent ou non 
extern String logs; // logs
extern byte security; // sécurité
extern byte addr[8]; 
extern float previous_celsius; // température précédente

float CheckTemperature(String label, byte deviceAddress[12]);

/// @brief / task executé toute les n secondes pour publier la température ( voir déclaration task dans main )
void mqttdallas() {
    if (mqtt_config.mqtt && present == 1 ) {
        sysvar.celsius=CheckTemperature("Inside : ", addr); 
        // arrondi à 1 décimale 
        if ( (sysvar.celsius == -127.00) || (sysvar.celsius == -255.00) ) {
         sysvar.celsius=previous_celsius;
        }
        else { 
        sysvar.celsius = (roundf(sysvar.celsius * 10) / 10 ) + 0.1; // pour les valeurs min
        }

        if ( sysvar.celsius != previous_celsius ) {
        // envoie des infos en mqtt dans ce cas
        mqtt(String(config.IDXTemp), String(sysvar.celsius));
        if ( mqtt_config.HA ) { device_temp.send(String(sysvar.celsius)); }
        logs += "Dallas temp : "+ String(sysvar.celsius) +"\r\n";
        }
        //}
    } 
    //// détection sécurité température
        if  ( sysvar.celsius > config.maxtemp ) {
            // coupure du dimmer
            sysvar.puissance=0;
        if ( mqtt_config.HA ) { device_dimmer.send("0"); }
        }
  
  previous_celsius=sysvar.celsius;
}
 
    //***********************************
    //************* récupération d'une température du 18b20
    //***********************************

float CheckTemperature(String label, byte deviceAddress[12]){
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print(label);
  if ( (tempC == -127.00) || (tempC == -255.00) ) {
    
    //// cas d'une sonde trop longue à préparer les valeurs 
    delay(187); /// attente de 187ms ( temps de réponse de la sonde )
    tempC = sensors.getTempC(deviceAddress);
      if ( (tempC == -127.00) || (tempC == -255.00) ) {
      Serial.print("Error getting temperature");
      logs += loguptime() + "Dallas on error\r\n";
      }
  } else {
    Serial.print(" Temp C: ");
    Serial.println(tempC);
    return (tempC); 
   
  }  
  return (tempC); 
}
#endif