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

#include <ESP8266WiFi.h>
// #include <WiFiClient.h>

//ADVANCE
#include "Hotspot.h"

// #include "Settings.h"
// #include "WebRequest.h"
// #include "WebServer.h"
// #include "WifiClient.h"

File myFile;

bool access(String uid) {
  Display_Show(" Syntry Mini v1", " AUTHENTICATING");
  Buzzer_Play(1, 700, 50);
  Serial.println("UID: " + uid);

  String filepath = "users/" + uid;
  myFile = SD.open(filepath);

  if (!myFile) {
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

bool add(String uid) {
  String filepath = "users/" + uid;
  myFile = SD.open(filepath, FILE_WRITE);

  if (myFile) {
    myFile.println("Start of Logs.");
    myFile.close();
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
  myFile = SD.open(filepath);

  if (myFile) {
    myFile.close();
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
  Config_Init();
  Relay_Init();
  Rfid_Init();
  SDCard_Init();

  // myFile = SD.open("/");
  // printDirectory(myFile, 0);
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  // myFile = SD.open("test.txt", FILE_WRITE);
  // // if the file opened okay, write to it:
  // if (myFile) {
  //   Serial.print("Writing to test.txt...");
  //   myFile.println("testing 1, 2, 3.");
  //   // close the file:
  //   myFile.close();
  //   Serial.println("done.");
  // } else {
  //   // if the file didn't open, print an error:
  //   Serial.println("error opening test.txt");
  // }
  // myFile = SD.open("/");
  // printDirectory(myFile, 0);

  Hotspot_broadcast();

  Display_Init();
  Display_Show(" Syntry Mini v1", "by BytesCrafter");
  Display_Show(" Syntry Mini v1", " TAP YOUR CARD");

  Buzzer_Play(3, 1200, 50);
}

void loop() {
  Hotspot_loop();
  Rfid_Listen(&catch_Rfid);
}