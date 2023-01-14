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
//#include <WiFiClient.h>

//NTP CLIENT
#include <NTPClient.h>
#include <WiFiUdp.h>
const long utcOffsetInSeconds = 28800; // +8hrs
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//ADVANCE
#include "WifiClient.h"
#include "Hotspot.h"

// #include "Settings.h"
// #include "WebRequest.h"
// #include "WebServer.h"

File myFile;

bool access(String uid) {
  if(uid == RST_KEYTAG) {
    SD.remove("users/admin");
    Display_Show(" Syntry Mini v1", "RESET ADMIN PASS");
    Buzzer_Play(1, 1000, 100); delay(2000);
    Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    return true;
  }

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

  String timing = String(timeClient.getHours())+":"+String(timeClient.getMinutes())+":"+String(timeClient.getSeconds());
  Serial.println(timing);
  Display_Show(timing, " TAP YOUR CARD");
  delay(3000);

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

  WiFi.mode(WIFI_AP_STA); 
  Hotspot_broadcast();

  //Check first the wifi.
  String wifiname;
  String wnpath = "settings/wifiname";
  File wnfile = SD.open(wnpath);
  if (wnfile) {
    wifiname = wnfile.readString();
  }

  String wifipass;
  String wppath = "settings/wifipass";
  File wpfile = SD.open(wppath);
  if (wpfile) {
    wifipass = wpfile.readString();
  }

  if(wifiname != "" && wifipass != "") {
    WifiClient_connect("GuestNetwork", "ahm2022!");
    timeClient.begin();
  }
  delay(1000);

  Display_Init();
  Display_Show(" Syntry Mini v1", "by BytesCrafter");
  Buzzer_Play(3, 1200, 500);
}

void loop() {
  timeClient.update();

  if(millis() - initTime <= timeSpan) {
    return;
  }

  if(!isInit) {
    Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    isInit = true;
  }

  Hotspot_loop();
  Rfid_Listen(&catch_Rfid);
}