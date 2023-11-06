#ifndef CONFIG
#define CONFIG

/**
 * MQTT credentials
 */

 
// #define MQTT_SEND true  //// set to false if not MQTT server is present ( automatic false on AP mode )---> configuration in web page --> to delete 01/23
//#define MQTT_USER ""   //// not used, use mqtt.json file  --> to delete 01/23
//#define MQTT_PASSWORD ""  //// not used, use mqtt.json file --> to delete 01/23

#define VERSION "Version 20231106" 

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
#define GRIDFREQ 50 ///PWM frequency
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
//#define outputPin2  27 /// désactivé pour le moment ne comprneds pas pourquoi c'est arrivé dans le code. 
#define zerocross  33
#define ONE_WIRE_BUS 4 
#define RELAY1 25 
#define RELAY2 26 
#define COOLER 17  
#endif

#ifdef ESP32ETH
#undef outputPin
#undef outputPin2
#undef zerocross
#undef ONE_WIRE_BUS
#undef RELAY1
#undef RELAY2
#undef COOLER

#define outputPin    15
//#define outputPin2   14 /// désactivé pour le moment
#define zerocross    36
#define ONE_WIRE_BUS 12 
//#define RELAY1       2 
//#define RELAY2       4 
#define COOLER       5

#define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT
#define ETH_POWER_PIN   16
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_ADDR        1
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
#endif

/// Trigger for temp limit in percent
#define TRIGGER 10   /// 
#define TIMERDELAY 5 // delay before switch off

#define LOG_MAX_STRING_LENGTH 512 // taille max des logs stockées

#define FACTEUR_REGULATION 0.7 // facteur de régulation pour l'enfant

/// activation mode debug
//#define Debug
  #ifdef Debug
    #define DEBUG_PRINTLN(x) Serial.println(x)
  #else
    #define DEBUG_PRINTLN(x)
  #endif

#endif

