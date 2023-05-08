#ifndef CONFIG
#define CONFIG

/**
 * MQTT credentials
 */

 
// #define MQTT_SEND true  //// set to false if not MQTT server is present ( automatic false on AP mode )---> configuration in web page --> to delete 01/23
//#define MQTT_USER ""   //// not used, use mqtt.json file  --> to delete 01/23
//#define MQTT_PASSWORD ""  //// not used, use mqtt.json file --> to delete 01/23

#define VERSION "Version 20230508" 

/// default configuration for Dimmer with Power supply and D1 Mini on the board ( default : D0 - D1 )
#ifdef  POWERSUPPLY
#define outputPin  D0 
#define zerocross  D1 // for boards with CHANGEBLE input pins
//#define outputPin  4
//#define zerocross  5 // for boards with CHANGEBLE input pins
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
#define ONE_WIRE_BUS D7 // Dallas 
#define RELAY1 D3  // Spécial pour relay commandé 
#define RELAY2 D2  //
#define COOLER  D8   /// 0 : off  -> 1 : On --> need a dry contact or opto
#endif

//// configuration for Dimmer with Power supply and D1 Mini on the board and need TTL USB ( https://fr.aliexpress.com/item/1005003365062050.html ) 
#ifdef  POWERSUPPLY2022
#define outputPin  4
#define zerocross  5 // for boards with CHANGEBLE input pins
#define COOLER  D7   /// 0 : off  -> 1 : On --> need a dry contact or opto

/// Dallas 
#define ONE_WIRE_BUS 12  
#define GND_PIN 14 
#define POS_PIN 02

#endif

#ifdef  SSR  /// même pin que le stand alone 
#define JOTTA  D1 // for boards with CHANGEBLE input pins
#define ONE_WIRE_BUS D7 // dallas 
#define GRIDFREQ 90 ///PWM frequency
#define outputPin  D5 
#define zerocross  D6
#define COOLER  D8   /// 0 : off  -> 1 : On --> need a dry contact or opto
#define RELAY1 D3  // Spécial pour relay commandé 
#define RELAY2 D2  //
#endif

//// Dallas Configuration for Dimmer with Power supply and D1 Mini on the board
#ifdef  POWERSUPPLY
#define ONE_WIRE_BUS D2 // previously D2 was not working on old robotdyn card
#define COOLER  D7   /// 0 : off  -> 1 : On --> need a dry contact or opto
#endif

#ifdef ESP32
#define outputPin  14
#define outputPin2  27
#define zerocross  33
#define ONE_WIRE_BUS 4 
#define RELAY1 25 
#define RELAY2 26 
#define COOLER 17  
#endif

/// Trigger for temp limit in percent
#define TRIGGER 10   /// 
#define TIMERDELAY 5 // delay before switch off

#define LOG_MAX_STRING_LENGTH 254 // taille max des logs stockées
#endif

