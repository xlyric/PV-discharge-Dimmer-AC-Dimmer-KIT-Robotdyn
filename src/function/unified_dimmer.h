#ifndef UNIFIED_DIMMER_FUNCTIONS
#define UNIFIED_DIMMER_FUNCTIONS

// le but de cette fonction est de centraliser les commandes de dimmer  ( robotdyn et SSR ) de façon uniforme 

#include <Arduino.h>
#ifdef ROBOTDYN
  #include "function/dimmer.h"
#else
    #include "function/jotta.h"
#endif



// @brief  structure pour uniformiser les commandes de puissances entre robotdyn et SSR
struct gestion_puissance
{
public:float power;

// setter
void set_power(float power){
  if ( sysvar.celsius > config.maxtemp ) { power = 0; } /// si la température est supérieur à la température max on coupe tout
  else if ( power > config.maxpow )  { power = config.maxpow; }

  /// vérification de la température 
  
  this->power = power;
  /// pour le SSR
  #ifdef SSR_ZC
    ssr_burst.set_power(int(power));
  #endif
 
  #ifdef SSR_RANDOM
    jotta_command(int(power));
  #endif

  /// pour le dimmer robotdyn
  #ifdef ROBOTDYN
    dimmer.setPower(int(power));
  #endif
   
}

//getter
int get_power(){
  // pour le ssr 
    #ifdef SSR_ZC
      power = ssr_burst.get_power();
    #endif

    #ifdef SSR_RANDOM
      power = sysvar.puissance;
    #endif

    // pour le dimmer robotdyn
    #ifdef ROBOTDYN
      power = dimmer.getPower();
    #endif
  return power;
}

/// @brief  migration des function de coupures des dimmers Robotdyn 

void dimmer_on()
{
  #ifdef ROBOTDYN
    if (dimmer.getState()==0) {
      dimmer.setState(ON);
      logging.Set_log_init("Dimmer On\r\n");
      delay(50);
    }
    #ifdef outputPin2
      if (dimmer2.getState()==0) {
        dimmer2.setState(ON);
        logging.Set_log_init("Dimmer2 On\r\n");
        delay(50);
      }  
    #endif
  #endif
}

void dimmer_off()
{
  #ifdef ROBOTDYN
    if (dimmer.getState()==1) {
      dimmer.setPower(0);
      dimmer.setState(OFF);
      logging.Set_log_init("Dimmer Off\r\n");
      delay(50);
    }
    #ifdef outputPin2 /// désactivé pour le moment
      dimmer2.setPower(0);
      dimmer2.setState(OFF);
      logging.Set_log_init("Dimmer2 Off\r\n");
      delay(50);
    #endif
  #endif
}

};

#endif