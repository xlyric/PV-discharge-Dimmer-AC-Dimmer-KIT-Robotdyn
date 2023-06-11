#ifndef TASK_COOLER 
#define TASK_COOLER

#include "Arduino.h"

extern System sysvar;
extern Config config;
extern Mqtt mqtt_config;
extern String logs;
extern bool AP; // mode point d'accès
extern HA device_cooler;

void cooler() {

                      
    bool cooler_change = sysvar.cooler ;

    /// controle du cooler 
    if (config.dimmer_on_off == 1){
        if ( sysvar.puissance > config.minpow && sysvar.celsius < config.maxtemp ) {
            sysvar.cooler = 1;
        } else {
            sysvar.cooler = 0;
        }
    } 
    else {
        sysvar.cooler = 0;
    }

    if ( cooler_change != sysvar.cooler ) {
        if ( sysvar.cooler == 1 ) {
        digitalWrite(COOLER, HIGH);
        // envoie mqtt
        if ( mqtt_config.HA ) {  device_cooler.send(stringbool(true));  }
        } else {
        digitalWrite(COOLER, LOW);
        if ( mqtt_config.HA ) {  device_cooler.send(stringbool(false));  }
        }
    }

 // pas besoin de tempo pour l'arret, vu que c'est toute les 15 secondes la task 
}

#endif

