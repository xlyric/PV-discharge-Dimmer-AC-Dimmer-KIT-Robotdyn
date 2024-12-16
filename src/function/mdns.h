#ifndef FUNCTION_MDNS
#define FUNCTION_MDNS

#include "../config/enums.h"
#include "../config/config.h"

void mdns_hello(String esp_name) {
    
    if (!MDNS.begin(esp_name.c_str())) {   
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
     }

    MDNS.addService("sunstain", "tcp", 80);
    MDNS.addServiceTxt("sunstain", "tcp", "name", esp_name.c_str());
    MDNS.addServiceTxt("sunstain", "tcp", "version", VERSION);
    MDNS.addServiceTxt("sunstain", "tcp", "compilation", COMPILE_NAME);
    MDNS.addServiceTxt("sunstain", "tcp", "fonction", "dimmer");
    MDNS.addServiceTxt("sunstain", "tcp", "url", "https://www.sunstain.fr");
    MDNS.addServiceTxt("sunstain", "tcp", "update_url", "https://ota.apper-solaire.org/ota.php");

}



#endif