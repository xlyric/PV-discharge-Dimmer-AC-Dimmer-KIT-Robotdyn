#ifndef DIMMER_FUNCTIONS
#define DIMMER_FUNCTIONS

///regroupement des fonctions de dimmer

#include <Arduino.h>
#include "mqtt.h"
#include <RBDdimmer.h>

#ifdef ESP32
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

extern struct System sysvar;
extern struct Config config;  
extern dimmerLamp dimmer;
#ifdef outputPin2
extern dimmerLamp dimmer2;
#endif

///déclaration des fonctions
void dimmer_on();
void dimmer_off();

/// fonction pour mettre en pause ou allumer le dimmer 
void dimmer_on()
{
  if (dimmer.getState()==0) {
    dimmer.setState(ON);
    logs +="Dimmer On\r\n"; 
    delay(50);
    }
  #ifdef outputPin2
    if (dimmer2.getState()==0) {
      dimmer2.setState(ON);
      logs +="Dimmer2 On\r\n"; 
      delay(50);
    }  
  #endif
}

void dimmer_off()
{
  if (dimmer.getState()==1) {
    dimmer.setPower(0);
    dimmer.setState(OFF);
    logs +="Dimmer Off\r\n"; 
    digitalWrite(COOLER, LOW);
    delay(50);
    }
  #ifdef outputPin2
    dimmer2.setPower(0);
    dimmer2.setState(OFF);
    logs +="Dimmer2 Off\r\n"; 
    delay(50);
  #endif
}


/*
void envoie_vers_enfant(int puissance);
void child_communication(int delest_power); 
void traitement_puissance_dispo();

/// @brief //fonction de communication avec la carte fille envoie des valeurs de puissance 
/// il doit être envoyé la totalité de la puissance disponible
void envoie_vers_enfant(int puissance,int charge) {
        //// si le champs n'est pas vide on envoie la valeur de puissance à la carte fille
        if ( strcmp(config.child,"off") != 0 ) {
              
            /// cas du mode equal 
            if ( strcmp(config.mode,"equal") == 0) { 
                    /// si dimmer à fond on delest tout
                    if (puissance >= config.maxpow) { child_communication(charge,false); }
                    /// si température de sécurité on delest tout
                    else if (sysvar.security == 1) { child_communication(charge,false); }
                    /// stop de sécurité 
                    else if ( puissance == 0 ) { child_communication(0,false); }
                    /// sinon 50% de la puissance
                    else { child_communication(charge/2,false); }
            }

                /// cas du mode delester     
            if ( strcmp(config.mode,"delester")) { 
                    /// si dimmer à fond on delest tout
                    if (puissance >= config.maxpow){
                        child_communication(charge,false); 
                    }
                    /// si température de sécurité on delest tout
                    else if (sysvar.security == 1) { child_communication(charge,false); }
                    /// si puissance négative et que la puissance cumulée est supérieure à la puissance de charge on retire la puissance de charge
                    else if (charge < 0 && sysvar.puissance_cumul > config.charge ) { child_communication(charge,false); }
                    /// sinon on envoie un 0 
                    else { child_communication(0,false); }

            } 
        }

}

//// envoie des commandes HTTP vers la carte fille 

void child_communication(int delest_power){
  int instant_power ;
  String baseurl; 
  if (config.dispo != 0 ) { instant_power = config.dispo; } 
  else { instant_power = delest_power*config.charge/100   ; }  /// ça posera problème si il y a pas de commandes de puissance en W comme le 2eme dimmer se calque sur la puissance du 1er 

  baseurl = "/?POWER=" + String(delest_power) + "&puissance=" + instant_power ; http.begin(domotic_client,config.child,80,baseurl); 
  http.GET();
  http.end(); 
  logs += "child at " + String(delest_power) + " " + String(instant_power) +  "\r\n";
}

void traitement_puissance_dispo(){
    /// puissance admissible par le dimmer
    sysvar.puissancemax = config.maxpow*config.charge/100;
    /// puissance actuelle dans le dimmer   
    int actual_dimmer = dimmer.getPower();
    sysvar.puissancewatt = actual_dimmer*config.charge/100;
    /// cas normal de fonctionnement on module la charge en fonction de la puissance dispo
    if (sysvar.puissancewatt < sysvar.puissancemax ) { 
        /// si la puissance dispo est positive et dépasse la capacité du dimmer alors on délestera la puissance en trop
        if ((sysvar.puissancewatt +  sysvar.puissance_dispo)  > sysvar.puissancemax) { 
            dimmer.setPower(config.maxpow);
            sysvar.puissance_dispo = sysvar.puissance_dispo - (sysvar.puissancemax - sysvar.puissancewatt);
        }
        /// si la puissance dispo est positive on augmente la puissance du dimmer
        else { 
            dimmer.setPower(actual_dimmer + sysvar.puissance_dispo/config.charge*100); 
            sysvar.puissance_dispo = 0; 
            actual_dimmer = dimmer.getPower();
            sysvar.puissancewatt = actual_dimmer*config.charge/100;
        }
     }
    /// si la puissance dispo est négative et 
    /// extinction du dimmer si commande = 0 
    if (sysvar.puissance==0) { 
        sysvar.puissance_dispo=0; 
    }
    
}
*/


#endif