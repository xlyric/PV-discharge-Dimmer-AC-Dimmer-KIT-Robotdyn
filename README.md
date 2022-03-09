# # Distant dimmer for discharge PV surplus 

[TOC]

I remaster the distant dimmer with the new ALL in dimmer from Robotdyn ( and support 8A !! ) 


<a href="https://robotdyn.com/diy-iot-ac-dimmer-kit-esp8266-wi-fi-d1-mini-for-ac-110-240v-dimming-control.html"><img src="https://robotdyn.com/pub/media/catalog/product/cache/3c7e6817bfdaeedae5fc1f56c1800828/i/o/iotdimmer_angle2_esp_logo.jpg" align="left" height="300"  ></a>

# Installation

make a git clone of the project and use Visual Studio Code for build and upload to the board. 
by default, D0 and D1 is used and solt on the board. 
D2 can be used by a Dallas 18b20, I explaint after how to make the small board

![Alt text](./images/front.png) 

# USE

at the first start, the wemos use the Wifimanager for configure the Wifi. 

```mermaid
sequenceDiagram
Start -->> New Wifi : 
New Wifi -->> Configure: 192.198.4.1
Configure -->> Start : wifi configured 
Start  -->> Run : Open Website and Dim
```

# CHANGE POWER ( for control )
for change power use the web site : 
Control :  http://IP/?POWER=xx
xx max = 99 

```mermaid
sequenceDiagram
Website -->> Dimmer : ?POWER=x
Dimmer -->> Website : Apply
```

you can send the command by you domotic server or your pv router 

# Note on Robotdyn librairie
with actual version of arduino GUI or VS, the librairie not working
I modify the librairie and is called in the lib_deps variable
lib_deps = https://github.com/xlyric/RBDDimmer

and called by plateformio.ini file

# Create the Dallas board

on a test plate, solder the long female pin from your Wemos D1 mini 

![Alt text](./images/1.jpg) 

( I cut the 3rd pin ( RX ) I use the place for GND connector ) 

solder a 5.6kOhms resistance between 3.3V hole ( first hole ) 
and D2 ( last hole ) 
and sold 3.3V - GND and D2 to your Dallas 18b20 
or add connector 

![Alt text](images/3.jpg) 

you can now connect the board to the main board 

![Alt text](./images/4.jpg)
![Alt text](./images/5.jpg)

# Update

You can update your firmware directly by the url /update 
it's open OTA web page, and you can push the firmware or filesystem. 

for generating firmware, on Visualstudio/ plateform IO, use the build option 
the build firmware is stored at .pio\build\d1_mini\firmware.elf folder 
