#ifndef TASK_GET_POWER
#define TASK_GET_POWER

#include "Arduino.h"
#include <ArduinoJson.h>


extern System sysvar;
extern Config config;

extern WiFiClient domotic_client;
extern HTTPClient http;


void get_dimmer_child_power (){
    /// récupération de la puissance du dimmer enfant en http
     if (!strcmp(config.mode, "off") == 0) {
        String baseurl; 
        baseurl = "/state";
        http.begin(domotic_client, String(config.child),80,baseurl);   
        
        int httpResponseCode = http.GET();
        String dimmerstate = "0"; 
        dimmerstate = http.getString();
        http.end();
        
        if (httpResponseCode==200) {
            DynamicJsonDocument doc(128);
            deserializeJson(doc, dimmerstate);
            //int ptotal = doc["Ptotal"];
            
            sysvar.puissance_cumul = doc["Ptotal"];
  
            Serial.println(sysvar.puissance_cumul);
        }
        else {
            sysvar.puissance_cumul = 0;
        }
     }

}


#endif