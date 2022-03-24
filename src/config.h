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
#define COOLER  D5   /// 0 : off  -> 1 : On --> need a dry contact or opto
#define TIMERDELAY 30 // delay before switch off

#endif

