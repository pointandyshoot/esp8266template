# esp8266template
Arduino sketch template for ESP8266, start your project with OTA, Wifi configuration and Adafruit IO already set up.
Get to programming your project faster!

Includes the following functions:
   ArduinoOTA  - over the air update of code
   WifiManager - ESP8266 keeps a log of APs it has connected to. If it can't find one it starts 
                 a AP, you connect with your phone and tell it the new wifi details. It will remember
                 between OTA updates.
                 https://github.com/tzapu/WiFiManager
   SimpleTimer - useful for calling events at rough time intervals (no interrupts)
   AdafruitIO  - update data from your sensor to AdafruitIO or send commands to the ESP8266 with an
                 easy to make interface.
   DeepSleep   - not a library, but the code to put the ESP8266 is set up in similar format. Good
                 for conserving battery power when that's an issue. If your ESP8266 has a power LED
                 consider disconnecting it to extend battery life more.
           NOTE: To use sleep you need to connect XPD to RST
                 Essentially restarts on wakeup (runs setup() etc)
