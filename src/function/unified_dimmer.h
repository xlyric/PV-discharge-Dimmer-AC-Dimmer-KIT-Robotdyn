#ifndef UNIFIED_DIMMER_FUNCTIONS
#define UNIFIED_DIMMER_FUNCTIONS

// le but de cette fonction est de centraliser les commandes de dimmer  ( robotdyn et SSR ) de façon uniforme

#include <Arduino.h>
#ifdef ROBOTDYN
  #include "function/dimmer.h"
  extern dimmerLamp dimmer;
  extern dimmerLamp dimmer2;
  extern dimmerLamp dimmer3;
#else
  #include "function/jotta.h"
#endif


// @brief  structure pour uniformiser les commandes de puissances entre robotdyn et SSR
struct gestion_puissance {
  public:float power;

  // setter
  void set_power(float is_set_power){
    if (sysvar.celsius[sysvar.dallas_maitre]> config.maxtemp ) {
      /// si la température est supérieur à la température max on coupe tout
      is_set_power = 0;
    } else if ( is_set_power > config.maxpow ) {
      is_set_power = config.maxpow;
    }

    /// vérification de la température
    this->power = is_set_power;
    /// pour le SSR
    #ifdef SSR_ZC
      ssr_burst.set_power(int(is_set_power));
    #endif

    #ifdef SSR_RANDOM
      jotta_command(int(is_set_power));
    #endif

    /// pour le dimmer robotdyn
    #ifdef ROBOTDYN
      // On transforme la puissance totale à envoyer aux dimmers en watts pour mieux les répartir entre les 3 SSR
      // Meilleure précision en float
      config.calcul_charge();
      /// eviter les divisions par 0
      float tmp_pwr_watt = 0;
      if ( int(is_set_power) >= 0 ) { tmp_pwr_watt = is_set_power * config.charge / 100; }

      int dimmer1_pwr = 0;
      int dimmer2_pwr = 0;
      int dimmer3_pwr = 0;

      // Calcul de la puissance à envoyer à chaque dimmer
      if (tmp_pwr_watt <= config.charge1){ // Un seul dimmer à fournir
        if ( tmp_pwr_watt > 0 ) {
          dimmer1_pwr = tmp_pwr_watt * 100 / config.charge1 ;
        } else {
          dimmer1_pwr = 0;
        }
        dimmer2_pwr = 0;
        dimmer3_pwr = 0;
      }

      else if (tmp_pwr_watt <= (config.charge1+config.charge2)){ // 2 dimmers à fournir
        if (config.charge1 != 0) { dimmer1_pwr = 100; } // Permet d'avoir le dimmer1 configuré à 0 dans l'interface web
        if (config.charge2 != 0) { dimmer2_pwr = (tmp_pwr_watt - config.charge1 ) * 100 / config.charge2 ;} // Permet d'avoir le dimmer2 configuré à 0 dans l'interface web
        dimmer3_pwr = 0;
      }
      else { // Les 3 dimmers à fournir
        if (config.charge1 != 0) { dimmer1_pwr = 100; } // Permet d'avoir le dimmer1 configuré à 0 dans l'interface web
        if (config.charge2 != 0) { dimmer2_pwr = 100; } // Permet d'avoir le dimmer2 configuré à 0 dans l'interface web
        if (config.charge3 != 0) { dimmer3_pwr = (tmp_pwr_watt - (config.charge1 + config.charge2) ) * 100 / config.charge3; } else { dimmer3_pwr = 0;}
      }

      Serial.println("power1" + String(dimmer1_pwr));
      // Application de la puissance à chaque dimmer
      // Dimmer1
      if ( dimmer1_pwr != dimmer.getPower() ) {
        if (dimmer1_pwr == 0 && dimmer.getState()==1) {
          dimmer.setPower(0);
          dimmer.setState(OFF);
          logging.Set_log_init("Dimmer1 Off\r\n");
          delay(50);
        }
        else if (dimmer1_pwr != 0 && dimmer.getState()==0) {
          dimmer.setState(ON);
          logging.Set_log_init("Dimmer1 On\r\n");
          delay(50);
          dimmer.setPower(dimmer1_pwr);
        }
        else { dimmer.setPower(dimmer1_pwr); }
      }

      #ifdef outputPin2
        // Dimmer2
        if ( dimmer2_pwr != dimmer2.getPower() ) {
          if (dimmer2_pwr == 0 && dimmer2.getState()==1) {
            dimmer2.setPower(0);
            dimmer2.setState(OFF);
            logging.Set_log_init("Dimmer2 Off\r\n");
            delay(50);
          }
          else if (dimmer2_pwr != 0 && dimmer2.getState()==0) {
            dimmer2.setState(ON);
            logging.Set_log_init("Dimmer2 On\r\n");
            delay(50);
            dimmer2.setPower(dimmer2_pwr);
          }
          else { dimmer2.setPower(dimmer2_pwr); }
        }

        // Dimmer3
        if ( dimmer3_pwr != dimmer3.getPower() ) {
          if (dimmer3_pwr == 0 && dimmer3.getState()==1) {
            dimmer3.setPower(0);
            dimmer3.setState(OFF);
            logging.Set_log_init("Dimmer3 Off\r\n");
            delay(50);
          }
          else if (dimmer3_pwr != 0 && dimmer3.getState()==0) {
            dimmer3.setState(ON);
            logging.Set_log_init("Dimmer3 On\r\n");
            delay(50);
            dimmer3.setPower(dimmer3_pwr);
          }
          else { dimmer3.setPower(dimmer3_pwr); }
        }
      #endif

      logging.Set_log_init("dimmer 1: " + String(dimmer1_pwr) + "%\r\n" );
      #ifdef outputPin2
        logging.Set_log_init("dimmer 2: " + String(dimmer2_pwr) + "%\r\n" );
        logging.Set_log_init("dimmer 3: " + String(dimmer3_pwr) + "%\r\n" );
      #endif
    #endif
  }

  //getter
  float get_power(){
    // pour le ssr
    #ifdef SSR_ZC
      power = ssr_burst.get_power();
    #endif

    #ifdef SSR_RANDOM
      power = sysvar.puissance;
    #endif

    // pour le dimmer robotdyn
    #ifdef ROBOTDYN
      config.calcul_charge();
      int power1 = dimmer.getPower();
      #ifdef outputPin2
        int power2 = dimmer2.getPower();
        int power3 = dimmer3.getPower();
        power = ((float)(power1*config.charge1 + power2*config.charge2 + power3*config.charge3) / (float)config.charge) ;
      #else
        power = ((float)(power1*config.charge1 ) / (float)config.charge) ;
      #endif
    #endif
    return power;
  }

  /// @brief  migration des function de coupures des dimmers Robotdyn
  void dimmer_off() {
    #ifdef ROBOTDYN
      if (dimmer.getState()) {
        dimmer.setPower(0);
        dimmer.setState(OFF);
        logging.Set_log_init("Dimmer1 Off\r\n");
        delay(50);
      }
      #ifdef outputPin2
        if (dimmer2.getState()) {
          dimmer2.setPower(0);
          dimmer2.setState(OFF);
          logging.Set_log_init("Dimmer2 Off\r\n");
          delay(50);
        }
      #endif
      #ifdef outputPin3
        if (dimmer3.getState()) {
          dimmer3.setPower(0);
          dimmer3.setState(OFF);
          logging.Set_log_init("Dimmer3 Off\r\n");
          delay(50);
        }
      #endif
    #endif

    #ifdef SSR_ZC
      ssr_burst.set_power(0);
    #endif
  }

};

#endif