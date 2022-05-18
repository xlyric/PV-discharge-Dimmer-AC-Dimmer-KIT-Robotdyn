#ifndef CONFIG
#define CONFIG

/**
 * MQTT credentials
 */
#define MQTT_USER ""
#define MQTT_PASSWORD ""


/// default configuration for Dimmer with Power supply and D1 Mini on the board ( default : D0 - D1 )
#ifdef  POWERSUPPLY
#define outputPin  D0 
#define zerocross  D1 // for boards with CHANGEBLE input pins
#endif

/*
/// default configuration for Dimmer with Power supply and D1 Mini on the board
#ifdef  POWERSUPPLY
#define outputPin  D6 
#define zerocross  D5 // for boards with CHANGEBLE input pins
#endif
*/

//// configuration for Standalone boards ( personnalisation )
#ifdef  STANDALONE
#define outputPin  D5 
#define zerocross  D6 // for boards with CHANGEBLE input pins
#endif

//// configuration for Dimmer with Power supply and D1 Mini on the board and need TTL USB ( https://fr.aliexpress.com/item/1005003365062050.html ) 
#ifdef  POWERSUPPLY2022
#define outputPin  4 
#define zerocross  5 // for boards with CHANGEBLE input pins
#define ONE_WIRE_BUS 14  /// Dallas on pin 14 
#define GND_PIN 16
#define POS_PIN 12
#endif

#ifdef  SSR
#define JOTTA  D4 // for boards with CHANGEBLE input pins
#define ONE_WIRE_BUS D2 // dallas 
#define GRIDFREQ 90 ///PWM frequency
#define outputPin  D0 
#define zerocross  D1
#endif

//// Dallas Configuration for Dimmer with Power supply and D1 Mini on the board
#ifdef  POWERSUPPLY
#define ONE_WIRE_BUS D2
#endif

//// Dallas Configuration for Standalone boards ( personnalisation )
#ifdef  STANDALONE
#define ONE_WIRE_BUS D7
#endif

/// Trigger for temp limit in percent
#define TRIGGER 10   /// 

//// pins for cooler
#define COOLER  D7   /// 0 : off  -> 1 : On --> need a dry contact or opto
#define TIMERDELAY 5 // delay before switch off

#endif

