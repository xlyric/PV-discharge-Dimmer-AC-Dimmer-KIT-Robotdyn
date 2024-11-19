#ifndef TASK_COOLER
#define TASK_COOLER

#include "Arduino.h"

extern System sysvar;
extern Config config;
extern Mqtt mqtt_config;
extern String logs;
extern bool AP; // mode point d'accès
extern HA device_cooler;
extern Programme programme;

unsigned long lastCoolerOffTime = 0; // NOSONAR
constexpr const unsigned long cooldownDuration = 60 * 1000; // 1 minute en millisecondes

void cooler() {

  bool cooler_change = sysvar.cooler;

  /// controle du cooler
  if (config.dimmer_on_off == 1) {
    if ( ( sysvar.puissance > config.minpow && sysvar.celsius[sysvar.dallas_maitre]< config.maxtemp &&
           sysvar.security == 0 ) || ( programme.run == true )) {
      sysvar.cooler = true;
    } else {
      sysvar.cooler = false;
    }
  }
  else {
    sysvar.cooler = false;
  }

  if ( cooler_change != sysvar.cooler ) {
    if ( sysvar.cooler == 1 ) {
      digitalWrite(COOLER, HIGH);
      // envoie mqtt
      if ( config.HA ) {  device_cooler.send(stringBool(true));  }
    } else {
      lastCoolerOffTime = millis();   // on enregistre le temps d'arret pour le cooldown
    }
  }

  if (sysvar.cooler == 0 && millis() - lastCoolerOffTime >= cooldownDuration && digitalRead(COOLER) == HIGH &&
      programme.run == false) {
    digitalWrite(COOLER, LOW);     // Éteindre le ventilateur après X secondes (cooldownDuration)

    if ( config.HA ) {  device_cooler.send(stringBool(false));  }
  }

  // pas besoin de tempo pour l'arret, vu que c'est toute les 15 secondes la task
}

#endif

