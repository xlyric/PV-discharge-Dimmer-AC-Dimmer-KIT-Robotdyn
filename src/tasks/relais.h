#ifndef TASK_RELAIS
#define TASK_RELAIS

#include "function/minuteur.h"
#include "Arduino.h"

extern Programme programme_relay1;
extern Programme programme_relay2;


/// création de la task de controle pour les relais ( 15 s )
void relais_controle() {
  #ifdef RELAY1
    /// activation du relai 1 si de seuil basse est remplie
    if (programme_relay1.start_seuil() && !programme_relay1.stop_seuil()) {
      digitalWrite(RELAY1, HIGH);
    }

    /// activation du relai 2 si de seuil basse est remplie
    if (programme_relay2.start_seuil() && !programme_relay2.stop_seuil()) {
      digitalWrite(RELAY2, HIGH);
    }

    /// déactivation du relai 1 si de seuil haute est remplie
    if (programme_relay1.stop_seuil()) {
      digitalWrite(RELAY1, LOW);
    }

    /// déactivation du relai 2 si de seuil haute est remplie
    if (programme_relay2.stop_seuil()) {
      digitalWrite(RELAY2, LOW);
    }

    /// désactivation Relai 1 si seuil temperature est atteint
    if (programme_relay1.stop_seuil_temp()) {
      digitalWrite(RELAY1, LOW);
    }

    /// désactivation relai 2 si seuil temperature est atteint
    if (programme_relay2.stop_seuil_temp()) {
      digitalWrite(RELAY2, LOW);
    }
  #endif
}

#endif