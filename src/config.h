#ifndef CONFIG
#define CONFIG

/**
 * MQTT credentials
 */
#define MQTT_USER ""
#define MQTT_PASSWORD ""


/// default configuration for Dimmer with Power supply and D1 Mini on the board
#ifdef  POWERSUPPLY
#define outputPin  D0 
#define zerocross  D1 // for boards with CHANGEBLE input pins
#endif


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

#endif

