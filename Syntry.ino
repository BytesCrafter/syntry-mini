/* wiring the MFRC522 to ESP8266 (ESP-12)
RST     = GPIO5    =  D1
SDA(SS) = GPIO4    =  D2
MOSI    = GPIO13   =  D7
MISO    = GPIO12   =  D6
SCK     = GPIO14   =  D5
GND     = GND
3.3V    = 3.3V
*/

/* wiring the SD-Card to ESP8266 (ESP-12)
SDA(SS) = GPI15    =  D8
MOSI    = GPIO13   =  D7
MISO    = GPIO12   =  D6
SCK     = GPIO14   =  D5
GND     = GND
3.3V    = 3.3V
*/

//DEFAULT
#include <SPI.h>
#include <Wire.h> 
#include <SD.h>
// #include <EEPROM.h>

//CUSTOM
#include "Config.h"
#include "Helper.h"

//INPUT
#include "Rfid.h"
#include "SDCard.h"
#include "Clock.h"

//OUTPUT
#include "Display.h"
#include "Relay.h"
#include "Light.h"
#include "Buzzer.h"

// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
// #include <WiFiClient.h>
// #include <ESP8266WebServer.h>
// #include "Settings.h"

// #include "Hotspot.h"
// #include "WebRequest.h"
// #include "WebServer.h"
// #include "WifiClient.h"

File myFile;

void setup() {
  Config_Init();
  Relay_Init();

  Rfid_Init();
  SDCard_Init();

  Display_Init();
  Display_Show(" Syntry Mini v1", "by BytesCrafter");
  Display_Show(" Syntry Mini v1", " TAP YOUR CARD");

  Buzzer_Play(3, 1200, 50);
}

int status = LOW;

void loop() {
  //delay(1000);
  //RtcDateTime now = Rtc.GetDateTime();
  //printDateTime(now);
  
  Rfid_Listen(&catch_Rfid);
}

bool catch_Rfid(String uid) {
  if(uid == "null") {
    return false;
  }

  Display_Show(" Syntry Mini v1", " AUTHENTICATING");
  Buzzer_Play(1, 700, 50);
  Serial.println("UID: " + uid);

  //TODO: Check if UID is within user list.
  delay(500);
  String compare = "45C1572A";

  if(uid != compare) {//User not known.
    Display_Show(" Syntry Mini v1", " ACCESS  DENIED");
    Buzzer_Play(1, 400, 50);
    delay(1000);

    Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    return false;
  }

  //TODO: Save log to SDCard
  //SDCard_Save("logs.txt", "User and Time Here"); //sHUTDOWN
  Display_Show(" Syntry Mini v1", " ACCESS GRANTED");
  Buzzer_Play(1, 900, 50);
  Relay_Open();

  Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
  return true;
}