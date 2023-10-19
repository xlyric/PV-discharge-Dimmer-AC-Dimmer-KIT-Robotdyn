#ifndef JOTTA_FUNCTIONS
#define JOTTA_FUNCTIONS

#include <Arduino.h>
#include "config/config.h"

/// code des tests SSR non random
#ifdef SSR_TEST

    struct SSR_BURST
    {
      /* data */
      public:char* sequence_result;
      private:int power;

      // getter puissance 
      public:int get_power(){
        return power;
      }

      //setter puissance 
      public:void set_power(int power){
        this->power = power;
        calcul(power);
      }

      public:void calcul(int puissance){
        double decimalValue; 
        decimalValue = round(puissance * 100) / 10000; // Arrondir à 2 décimales
        int denominator = 100; // Choisissez une valeur de dénominateur appropriée

        int numerator = decimalValue * denominator;
        int gcdValue = gcd(numerator, denominator);

        numerator /= gcdValue;
        denominator /= gcdValue;

        //Serial.print(decimalValue, 2); // Affichez le nombre décimal avec 4 décimales
        Serial.print("en fraction : ");
        Serial.print(numerator);
        Serial.print("/");
        Serial.print(denominator);
        
        float sequence = float(denominator) / float(numerator) ;
        Serial.print(" Sequence : ");
        Serial.println(sequence);
        // boucle de 100 occurences
        sequence_result = action_sequence(sequence);
      }

    /// @brief  fonction de calcul de la séquence d'ouverture et fermeture du SSR 
    /// @param sequence 
    /// @return 
      private:char* action_sequence(float sequence) {
        float boucle = sequence; 
        char* sequence_char = (char*)malloc(102); // Allouer de la mémoire pour le tableau de caractères
        
        for (int j = 1; j < 101; j++) {
          if (j >= boucle) {
            //Serial.print("+");
            sequence_char[j - 1] = '+';
            boucle = boucle + sequence;
          } else {
            sequence_char[j - 1] = '-';
            //Serial.print("-");
          }
        }
        
        sequence_char[101] = '\0'; // Terminer la chaîne avec un caractère nul
        //Serial.println("");
      // Serial.println(sequence_char);
        return sequence_char;
      }

    /// @brief  fonction de calcul du plus grand diviseur commun
      private:int gcd(int a, int b) {
        if (b == 0) {
          return a;
        }
        return gcd(b, a % b);
      } 

    };


    int sequence_timer = 0;

    extern SSR_BURST ssr_burst;

    void SSR_run(){
      if (ssr_burst.sequence_result[sequence_timer] == '+') {
        digitalWrite(JOTTA, HIGH);
      } else {
        digitalWrite(JOTTA, LOW);
      }

      sequence_timer++;

      if (sequence_timer == 100) {
        sequence_timer = 0;
      }

    }
#endif 
  // Configuration de la broche comme sortie

int time_sync = 0;  // Variable utilisée pour stocker le temps 
int frequency = 0;  // Variable utilisée pour stocker la fréquence du réseau
int time_tempo = 0;  // Variable utilisée pour stocker le temps temporaire
int jotta_pow = 0;  // Variable utilisée pour stocker la commande de dimmer
int avance_phase = 20;  // Variable utilisée pour stocker l'avance de phase

// 15 perfect pour moi, mais pour titi faut test 20
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
  #ifdef  SSR
	timer1_attachInterrupt(jotta_ISR);
	timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
	timer1_write(timeoutPinjotta); //100 us
  Serial.println("timer_init");
  #endif
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

void Jotta_burst_command(int command){
   /// calculer le temps de burst en fonction de la commande 
  #ifdef  SSR
   // calcul du nombre de modulation par seconde
  float nb_modulation = 100 / command; 
  
  

  


   #endif
}


#endif