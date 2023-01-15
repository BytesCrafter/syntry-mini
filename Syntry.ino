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
  MOSI    = GPIO13   =  D7 + 220K Resistor
  MISO    = GPIO12   =  D6
  SCK     = GPIO14   =  D5
  GND     = GND
  3.3V    = 3.3V
*/

//DEFAULT
#include <SPI.h>
#include <Wire.h> 
#include <EEPROM.h>
#include <SD.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

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

//NTP CLIENT
const long utcOffsetInSeconds = 28800; // +8hrs
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//#include "WebRequest.h"

//ADVANCE
#include "WifiClient.h"
#include "Hotspot.h"
// #include "Settings.h"

bool access(String uid) {
  if(uid == RST_KEYTAG) {
    SD.remove("users/admin");
    Display_Show(" Syntry Mini v1", "RESET ADMIN PASS");
    Buzzer_Play(1, 1000, 100); delay(1500);
    Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    return true;
  }

  Display_Show(" Syntry Mini v1", " AUTHENTICATING");
  Buzzer_Play(1, 700, 50);
  Serial.println("UID: " + uid);

  String filepath = "users/" + uid;
  File accessFile = SD.open(filepath);

  if (!accessFile) {
    accessFile.close();
    Display_Show(" Syntry Mini v1", " ACCESS  DENIED");
    Buzzer_Play(1, 400, 50);
    delay(500);

    Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    return false;
  }

  Display_Show(" Syntry Mini v1", " ACCESS GRANTED");
  Buzzer_Play(1, 900, 50); 
  delay(1000);

  String timing = String(timeClient.getFormattedTime());
  Display_Show(" Syntry Mini v1", "TIME: "+timing);
  
  //TODO: Save log to SDCard
  //SDCard_Save("logs.txt", "User and Time Here"); //sHUTDOWN

  Relay_Open();

  Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
  return true;
}

bool add(String uid) {
  String filepath = "users/" + uid;
  File addFile = SD.open(filepath, FILE_WRITE);

  if (addFile) {
    addFile.print(""); //TODO: Should be date.
    addFile.close();
    Display_Show(" Syntry Mini v1", " LOG: SAVED!");
    Buzzer_Play(1, 900, 50); delay(1000);
    return true;
  } else {
    Display_Show(" Syntry Mini v1", " LOG: FAILED!");
    Buzzer_Play(1, 300, 50); delay(1000);
    return false;
  }
}

bool remove(String uid) {
  String filepath = "users/" + uid;
  SD.remove(filepath);

  if (!SD.exists(filepath)) {
    Display_Show(" Syntry Mini v1", " LOG: DELETED!");
    Buzzer_Play(1, 900, 50); delay(1000);
    return false;
  } else {
    Display_Show(" Syntry Mini v1", " LOG: FAILED!");
    Buzzer_Play(1, 300, 50); delay(1000);
    return true;
  }
}

bool verify(String uid) {
  String filepath = "users/" + uid;
  File verifyFile = SD.open(filepath);

  if (verifyFile) {
    verifyFile.close();
    Display_Show(" Syntry Mini v1", " LOG: EXISTING!");
    Buzzer_Play(1, 900, 50); delay(1000);
    return true;
  } else {
    Display_Show(" Syntry Mini v1", " LOG: NOT FOUND!");
    Buzzer_Play(1, 300, 50); delay(1000);
    return false;
  }
}

bool catch_Rfid(String uid) {
  if(uid == "null") {
    return false;
  }

  if(Rfid_Status() == "access") {
    return access(uid);
  } else if(Rfid_Status() == "add") {
    return add(uid);
  } else if(Rfid_Status() == "remove") {
    return remove(uid);
  } else if(Rfid_Status() == "verify") {
    return verify(uid);
  } else {
    return false;
  }
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.print(entry.size(), DEC);
      time_t cr = entry.getCreationTime();
      time_t lw = entry.getLastWrite();
      struct tm * tmstruct = localtime(&cr);
      Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      tmstruct = localtime(&lw);
      Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    entry.close();
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  SPI.begin();
  
  Config_Init();
  Rfid_Init();
  SDCard_Init();

  Relay_Init();

  WiFi.mode(WIFI_AP_STA); 
  Hotspot_broadcast();

  Display_Init();
  Display_Show(" Syntry Mini v1", "by BytesCrafter");
  Buzzer_Play(3, 1200, 250);
}

bool didTryConnect = false;

void loop() {
  Config_Loop();

  Hotspot_loop();
  if(!didTryConnect) {
     WifiClient_connect();
    didTryConnect = true;
  }

  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
  }

  if(!isInit) {
    Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    isInit = true;
  }
  Rfid_Listen(&catch_Rfid);
}