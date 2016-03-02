// Basic template to start ESP8266 projects with useful features included.
// Tested on 4MB ESP8266, OTA may have issues with 1MB chips.

// Rev 0 1/3/2016 Initial comit

/*
 * Includes the following functions:
 *   ArduinoOTA  - over the air update of code
 *   WifiManager - ESP8266 keeps a log of APs it has connected to. If it can't find one it starts 
 *                 a AP, you connect with your phone and tell it the new wifi details. It will remember
 *                 between OTA updates.
 *   SimpleTimer - useful for calling events at rough time intervals (no interrupts)
 *   AdafruitIO  - update data from your sensor to AdafruitIO or send commands to the ESP8266 with an
 *                 easy to make interface.
 *   DeepSleep   - not a library, but the code to put the ESP8266 is set up in similar format. Good
 *                 for conserving battery power if that's an issue. If your ESP8266 has a power LED
 *                 consider disconnecting it to extend battery life more.
 *           NOTE: To use sleep you need to connect XPD to RST
 *                 Essentially restarts on wakeup (runs setup() etc)
 */

// Libraries required for OTA (Over the Air Update)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Libraries required for wifimanager (set wifi password without reflashing)
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager



// Libraries required for SimpleTimer
#include <SimpleTimer.h>
SimpleTimer timer;

// Libraries required for Adafruit IO

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "xxxxxxxxxxxx"
#define AIO_KEY         "xxxxxxxxxxxx"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_USERNAME, MQTT_PASSWORD);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>

const char DEBUG_FEED[] PROGMEM = AIO_USERNAME "/feeds/debug";
Adafruit_MQTT_Publish debug = Adafruit_MQTT_Publish(&mqtt, DEBUG_FEED);

const char PRESSURE_FEED[] PROGMEM = AIO_USERNAME "/feeds/pressure";
Adafruit_MQTT_Publish pressure = Adafruit_MQTT_Publish(&mqtt, PRESSURE_FEED);

const char TEMPERATURE_FEED[] PROGMEM = AIO_USERNAME "/feeds/temperature";
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE_FEED);

const char BATTERY_FEED[] PROGMEM = AIO_USERNAME "/feeds/battery";
Adafruit_MQTT_Publish battery = Adafruit_MQTT_Publish(&mqtt, BATTERY_FEED);

// Setup a feed called 'onoff' for subscribing to changes.
const char ONOFF_FEED[] PROGMEM = AIO_USERNAME "/feeds/onoff";
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, ONOFF_FEED);


// SleepTime

long defaultSleep = 60000000; // 60000000 = 60 seconds

// Additional libraries:
// *****




// *****

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void bootOTA() {
  Serial.println("<BOOT> OTA Update");
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//int i = 0;
void RepeatTask() {
//  debug.publish(i++);
}

void bootTimer() {
  Serial.println("<BOOT> SimpleTimer");
  // Valid calls:
  //  timer.setInterval(15000, RepeatTask);
  //  timer.setTimeout(10000, OnceOnlyTask);
  //  timer.setInterval(1000, DigitalClockDisplay);
  //  timer.setTimer(1200, TenTimesTask, 10);
}

void bootWifiManager() {
  Serial.println("<BOOT> WiFi Manager");
  //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    //wifiManager.autoConnect("AutoConnectAP");
    //or use this for auto generated name ESP + ChipID
    wifiManager.autoConnect();

    
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
}

void bootAdaIO() {
  Serial.println("<BOOT> AdafruitIO");
  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care of connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

void AdaIOLoop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
    }
  }

  // Now we can publish stuff!

 // debug.publish("AdaIOExec");
 // pressure.publish(bme.readPressure());
 // temperature.publish(bme.readTemperature());
  
/*  Serial.print(F("\nSending photocell val "));
  Serial.print(x);
  Serial.print("...");
  if (! photocell.publish(x++)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
*/
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

void deepSleep(long sleepTime) {
  ESP.deepSleep(sleepTime, WAKE_RF_DEFAULT); // Sleep for 60 seconds
}

void setup() {
  Serial.begin(115200);
  Serial.println("");

  //Template setup() calls
  bootWifiManager();
  bootOTA();
  bootTimer();
  bootAdaIO();

  //Custom setup() calls here







  //End of custom setup() calls
  Serial.println("<BOOT> Complete");
}

void loop() {
  // leave these at the top of loop()
  ArduinoOTA.handle();        // initiates OTA update capability
  timer.run();                // initiates SimpleTimer
  AdaIOLoop();                // initiates and maintains the Adafruit IO connection

  // loop code goes below











  // deepSleep(defaultSleep);  // put the ESP8266 to sleep for defaultSleep period of time. Remember to connect XPD to RST.

}
