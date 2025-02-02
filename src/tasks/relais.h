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
    digitalWrite(RELAY1, LOW); // correction bug de démarrage en GPIO 0
  }

  /// activation du relai 2 si de seuil basse est remplie
  if (programme_relay2.start_seuil() && !programme_relay2.stop_seuil()) {
    digitalWrite(RELAY2, HIGH);
  }

  /// déactivation du relai 1 si de seuil haute est remplie
  if (programme_relay1.stop_seuil()) {
    digitalWrite(RELAY1, HIGH); //correction bug de démarrage en GPIO 0
  }

  /// déactivation du relai 2 si de seuil haute est remplie
  if (programme_relay2.stop_seuil()) {
    digitalWrite(RELAY2, LOW);
  }

  /// désactivation Relai 1 si seuil temperature est atteint
  if (programme_relay1.stop_seuil_temp()) {
    digitalWrite(RELAY1, HIGH); //correction bug de démarrage en GPIO 0
  }

  /// désactivation relai 2 si seuil temperature est atteint
  if (programme_relay2.stop_seuil_temp()) {
    digitalWrite(RELAY2, LOW);
  }
    #endif
}

  #ifdef ESP32
    void relais_controle_32 ( void * parameter ) {
      while (true) {
        relais_controle();
        vTaskDelay(20020 / portTICK_PERIOD_MS); // Délai de 15 secondes pour ESP32
      }
    }
  #endif


#endif