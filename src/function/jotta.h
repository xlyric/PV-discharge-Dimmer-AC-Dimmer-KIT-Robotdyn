#ifndef JOTTA_FUNCTIONS
#define JOTTA_FUNCTIONS

#include <Arduino.h>
#include "config/config.h"

  // Configuration de la broche comme sortie

int time_sync = 0;  // Variable utilisée pour stocker le temps 
int frequency = 0;  // Variable utilisée pour stocker la fréquence du réseau
int time_tempo = 0;  // Variable utilisée pour stocker le temps temporaire
int jotta_pow = 0;  // Variable utilisée pour stocker la commande de dimmer
int avance_phase = 15;  // Variable utilisée pour stocker l'avance de phase
//>13
//<17
//volatile bool topDepart = false;  // Variable volatile utilisée comme indicateur de top départ

/*IRAM_ATTR void jotta_top() {
  //topDepart = true;  // Définir la variable d'indication de top départ à true
  #ifdef  SSR
  //frequency = 500 / (millis() - time_tempo);  // Calculer la fréquence du réseau  
  analogWrite(JOTTA, 0);
  delayMicroseconds(495);
  analogWrite(JOTTA, jotta_pow);
  //time_tempo = millis();
  detachInterrupt(digitalPinToInterrupt(zerocross));
  #endif

}*/

IRAM_ATTR void jotta_run(){
  #ifdef  SSR
      //frequency = time_sync; 
      time_sync = 0;
      //time_tempo = 0;
      //Serial.println("jotta_run");
      
  #endif

}


int timeoutPinjotta = 498; // 100 us
IRAM_ATTR void jotta_ISR()
{

  frequency = 100 - avance_phase; 
  time_tempo = 100- jotta_pow - avance_phase; 
  #ifdef  SSR
  if (jotta_pow == 0 ){  
    digitalWrite(JOTTA, LOW);
    //Serial.println("send 0");
    return;
  }
  
  if (time_sync == 100 - avance_phase){  
    digitalWrite(JOTTA, LOW);
    //frequency = time_tempo;
    return;
  }
  else if (time_sync >= (100-jotta_pow - avance_phase)){  
    digitalWrite(JOTTA, HIGH);
    //frequency = time_sync;
  }


  if (digitalRead(JOTTA)) {    time_tempo ++;  }

  time_sync ++;
  /*if (time_sync > 100) {
    time_sync = 0;
    //frequency = 0;
  }*/
  //timer1_write(timeoutPinjotta); //100 us

  #endif
}

void timer_init()
{
	timer1_attachInterrupt(jotta_ISR);
	timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
	timer1_write(timeoutPinjotta); //100 us
  Serial.println("timer_init");
}

void init_jotta() {
  #ifdef  SSR
  pinMode(JOTTA, OUTPUT);  // Configuration de la broche comme sortie
  //analogWriteFreq(50) ; 
  //analogWriteRange(100);
  //analogWrite(JOTTA, 0);
  jotta_pow = 0; // Initialiser la commande de dimmer à 0
  pinMode(zerocross, INPUT_PULLUP);  // Configuration de la broche comme entrée avec résistance de pull-up activée
  attachInterrupt(digitalPinToInterrupt(zerocross), jotta_run, FALLING);  // Attacher l'interruption à la broche D6 sur front descendant (FALLING)
  #endif
}

void jotta_command(int command) {
  #ifdef  SSR
  jotta_pow = command;
  
  //analogWrite(JOTTA, command);
  //attachInterrupt(digitalPinToInterrupt(zerocross), jotta_top, FALLING);  // Attacher l'interruption à la broche D6 sur front descendant (FALLING)
  //delay(20);
   #endif
}
/*
void jotta_sync(){
  attachInterrupt(digitalPinToInterrupt(zerocross), jotta_top, FALLING);
}
*/




#endif