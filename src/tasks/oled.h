#ifndef TASK_OLED
#define TASK_OLED

#include "Arduino.h"

#include <OLEDDisplay.h>
#include "OLEDDisplayUi.h"
#include "SSD1306Wire.h"
#include <WiFiUdp.h>

SSD1306Wire display(0x3c, 25, 27);
OLEDDisplayUi   ui( &display );
#define defaultFont ArialMT_Plain_16

time_t now;

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void display_routage(OLEDDisplay *display, OLEDDisplayUiState* state, short int x, short int y);
void display_temperature(OLEDDisplay *display, OLEDDisplayUiState* state, short int x, short int y);

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
void drawProgress(OLEDDisplay *display, int percentage, String label);

void init_ui(); 

const uint8_t activeSymbole[] PROGMEM = {
    B00000000,
    B00000000,
    B00011000,
    B00100100,
    B01000010,
    B01000010,
    B00100100,
    B00011000
};

const uint8_t inactiveSymbole[] PROGMEM = {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00011000,
    B00011000,
    B00000000,
    B00000000
};

// Adjust according to your language
const String WDAY_NAMES[] = {"DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM"};
const String MONTH_NAMES[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

struct Oled {

// init oled
    void init() {
        // Initialising the UI will init the display too.
        display.init();
        display.clear();
        display.display();
        //display.flipScreenVertically();
        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.setContrast(50, 100, 30);
    } 
 
// affichage de l'IP
    void display_ip() {
        display.setFont(ArialMT_Plain_10);
        String ip = WiFi.localIP().toString();
        String rssi = String(WiFi.RSSI());
        display.drawString(0, 0, ip);
        display.drawString(100, 0, rssi + "dB");
        display.setFont(defaultFont);
    }

    void wait_for_wifi(uint8_t counter) {
    display.clear();
    display.drawString(64, 10, "Connecting to WiFi");
    display.drawXbm(46, 30, 8, 8, counter % 3 == 0 ? activeSymbole : inactiveSymbole);
    display.drawXbm(60, 30, 8, 8, counter % 3 == 1 ? activeSymbole : inactiveSymbole);
    display.drawXbm(74, 30, 8, 8, counter % 3 == 2 ? activeSymbole : inactiveSymbole);
    display.display();
    }
   
};

Oled oled; 
FrameCallback frames[] = { display_temperature };
   int numberOfFrames = 1;
OverlayCallback overlays[] = { drawHeaderOverlay };
   int numberOfOverlays = 1;

// task oled et meteo 

void oled_task() {
    ui.update();
}

// affichage de l'heure
    void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    now = time(nullptr);
    struct tm* timeInfo;
    timeInfo = localtime(&now);
    char buff[16];

    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(ArialMT_Plain_10);
    String date = WDAY_NAMES[timeInfo->tm_wday];

    sprintf_P(buff, PSTR("%s, %02d/%02d/%04d"), WDAY_NAMES[timeInfo->tm_wday].c_str(), timeInfo->tm_mday, timeInfo->tm_mon+1, timeInfo->tm_year + 1900);
    display->drawString(64 + x, 5 + y, String(buff));
    display->setFont(ArialMT_Plain_24);

    sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
    display->drawString(64 + x, 15 + y, String(buff));
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    }
    
//// affichage des infos basse de l'écran
    void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
    now = time(nullptr);
    struct tm* timeInfo;
    timeInfo = localtime(&now);
    char buff[14];
    sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);

    display->setColor(WHITE);
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, 54, String(buff));
    display->setTextAlignment(TEXT_ALIGN_RIGHT);
    display->drawString(128, 54, String(unified_dimmer.get_power()*config.charge/100) );
    display->drawHorizontalLine(0, 52, 128);
    }


    void drawProgress(OLEDDisplay *display, int percentage, String label) {
    display->clear();
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(ArialMT_Plain_10);
    display->drawString(64, 10, label);
    display->drawProgressBar(2, 28, 124, 10, percentage);
    display->display();
    }

    // affichage de la température
    void display_temperature(OLEDDisplay *display, OLEDDisplayUiState* state, short int x, short int y) {

        display->setTextAlignment(TEXT_ALIGN_CENTER);
        display->setFont(ArialMT_Plain_24);
        int puissance_temp = unified_dimmer.get_power();
        String info_puissance = String(puissance_temp*config.charge/100) + "W";
        display->drawString(64 + x, 0 + y, String( info_puissance ));
        display->setFont(ArialMT_Plain_24);
        String inside_temp = String(sysvar.celsius[sysvar.dallas_maitre]) + "°C";
        display->drawString(64 + x, 24 + y, String( inside_temp ));
 
    }

    void display_routage(OLEDDisplay *display, OLEDDisplayUiState* state, short int x, short int y) {
        display->setTextAlignment(TEXT_ALIGN_CENTER);
        display->setFont(ArialMT_Plain_24);
        String info_puissance = String(unified_dimmer.get_power()) + "W";
        display->drawString(64 + x, 0 + y, String( info_puissance ));
    }


    void init_ui() {
        ui.setTargetFPS(5);

        ui.setActiveSymbol(activeSymbole);
        ui.setInactiveSymbol(inactiveSymbole);

        // TOP, LEFT, BOTTOM, RIGHT
        ui.setIndicatorPosition(BOTTOM);

        // Defines where the first frame is located in the bar.
        ui.setIndicatorDirection(LEFT_RIGHT);

        // You can change the transition that is used
        // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
        ui.setFrameAnimation(SLIDE_LEFT);

        ui.setFrames(frames, numberOfFrames);

        ui.setOverlays(overlays, numberOfOverlays);

        ui.init();

    }


#endif
