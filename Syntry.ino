/*
  Syntry Access - RFID Access Control System
  
  REQUIRED LIBRARIES (Install via Arduino Library Manager or GitHub):
  ==================================================================
  1. MFRC522 (RFID Reader)
     GitHub: https://github.com/miguelbalboa/rfid
     Library Manager: "MFRC522 by GithubCommunity"
  
  2. LiquidCrystal_I2C (LCD Display)
     GitHub: https://github.com/johnrickman/LiquidCrystal_I2C
     Library Manager: "LiquidCrystal I2C by Frank de Brabander"
  
  3. NTPClient (Network Time Protocol)
     GitHub: https://github.com/arduino-libraries/NTPClient
     Library Manager: "NTPClient by Fabrice Weinberg"
  
  4. RTC (Real Time Clock) - Optional
     GitHub: https://github.com/Makuna/Rtc/wiki
     Library Manager: "Rtc by Makuna"
  
  5. Adafruit Sensor (Sensor Abstraction) - Optional
     GitHub: https://github.com/adafruit/Adafruit_Sensor
     Library Manager: "Adafruit Unified Sensor"
  
  BUILT-IN LIBRARIES (Included with ESP8266 Core):
  - ESP8266WiFi
  - ESP8266WebServer
  - DNSServer
  - SPI, Wire, SD, EEPROM
  
  ESP8266 BOARD SETUP:
  ==================================================================
  Arduino IDE > File > Preferences > Additional Board Manager URLs:
  http://arduino.esp8266.com/stable/package_esp8266com_index.json
  
  Then: Tools > Board > Boards Manager > Search "ESP8266" > Install
  Select: Tools > Board > NodeMCU 1.0 (ESP-12E Module)

  ESP 8266 DIAGRAM https://lastminuteengineers.com/wp-content/uploads/iot/ESP8266-Pinout-NodeMCU.png
*/

/* wiring the MFRC522 to ESP8266 (042125) https://randomnerdtutorials.com/esp8266-nodemcu-mfrc522-rfid-reader-arduino/
  RST     = Not Connected (using software reset) Prev: GPIO16   =  D0
  SDA(SS) = GPIO02   =  D3
  MOSI    = GPIO13   =  D7
  MISO    = GPIO12   =  D6
  SCK     = GPIO14   =  D5
  GND     = GND
  3.3V    = 3.3V
*/

/* wiring the SD-Card to ESP8266 (042125) https://github.com/G6EJD/ESP8266-SD-Card-Reading-Writing/blob/master/ESP8266_D1_MicroSD_Test.ino
  ⚠️ WARNING: If SD Card powered by 5V, add 220Ω resistor on MISO line to protect ESP8266!
  CS      = GPIO02   =  D4
  MOSI    = GPIO13   =  D7
  MISO    = GPIO12   =  D6 + 220R Resistor (REQUIRED for 5V SD card!)
  SCK     = GPIO14   =  D5
  GND     = GND
  3.3V    = 3.3V (or 5V if using voltage regulator module)
*/

/* wiring the LCD L2C to ESP8266 (042125)
  SCL     = GPIO5   =  D1
  SDA     = GPIO4   =  D2
  GND     = GND
  3.3V    = 3.3V
*/

/* wiring the LIGHTS to ESP8266 (042125)
  LED RED = GPIO09   =  SD2
  LED GRN = GPIO08   =  SD1
  RELAY   = GPIO16   =  D0  (No boot restrictions!)
  BUZZ    = GPIO15   =  D8
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

bool access(String uid) {
  Display_Show(String(" ") + APP_NAME, " AUTHENTICATING");
  Buzzer_Play(1, 700, 50);
  Serial.println("UID: " + uid);

  Config_SelectSDCard();  // Select SD Card before file operation
  String filepath = "users/" + uid;
  File accessFile = SD.open(filepath);

  if (!accessFile) {
    Config_DeselectAll();  // Clean up SPI bus
    Display_Show(String(" ") + APP_NAME, " ACCESS  DENIED");
    Buzzer_Play(1, 400, 50);
    Serial.println("Access denied: User not found");
    delay(500);

    Display_Show(String(" ") + APP_NAME, " TAP YOUR CARD");
  } else {
    accessFile.close();
    Config_DeselectAll();  // Clean up SPI bus
    
    //String curTime = String(timeClient.getFormattedTime());
    //Display_Show(" " + APP_NAME, " TIME: "+curTime);
    
    Serial.println("Access granted: " + uid);
    //TODO: Save log to SDCard
    //SDCard_Save("logs.txt", "User and Time Here"); //sHUTDOWN

    Display_Show(String(" ") + APP_NAME, " ACCESS GRANTED");
    Buzzer_Play(1, 900, 50); 

    Relay_Open();
  }

  Display_Show(String(" ") + APP_NAME, " TAP YOUR CARD");
  return true;
}

bool add(String uid) {
  Config_SelectSDCard();
  String filepath = "users/" + uid;
  File addFile = SD.open(filepath, FILE_WRITE);

  if (addFile) {
    addFile.print(""); //TODO: Should be date.
    addFile.flush();
    addFile.close();
    Config_DeselectAll();
    Serial.println("User added: " + uid);
    Display_Show(String(" ") + APP_NAME, " LOG: SAVED!");
    Buzzer_Play(1, 900, 50); delay(1000);
    return true;
  } else {
    Config_DeselectAll();
    Serial.println("Failed to add user: " + uid);
    Display_Show(String(" ") + APP_NAME, " LOG: FAILED!");
    Buzzer_Play(1, 300, 50); delay(1000);
    return false;
  }
}

bool remove(String uid) {
  Config_SelectSDCard();
  String filepath = "users/" + uid;
  bool removed = SD.remove(filepath);
  bool exists = SD.exists(filepath);
  Config_DeselectAll();

  if (!exists && removed) {
    Serial.println("User removed: " + uid);
    Display_Show(String(" ") + APP_NAME, " LOG: DELETED!");
    Buzzer_Play(1, 900, 50); delay(1000);
    return true;
  } else {
    Serial.println("Failed to remove user: " + uid);
    Display_Show(String(" ") + APP_NAME, " LOG: FAILED!");
    Buzzer_Play(1, 300, 50); delay(1000);
    return false;
  }
}

bool verify(String uid) {
  Config_SelectSDCard();
  String filepath = "users/" + uid;
  File verifyFile = SD.open(filepath);

  if (verifyFile) {
    verifyFile.close();
    Config_DeselectAll();
    Serial.println("User exists: " + uid);
    Display_Show(String(" ") + APP_NAME, " LOG: EXISTING!");
    Buzzer_Play(1, 900, 50); delay(1000);
    return true;
  } else {
    Config_DeselectAll();
    Serial.println("User not found: " + uid);
    Display_Show(String(" ") + APP_NAME, " LOG: NOT FOUND!");
    Buzzer_Play(1, 300, 50); delay(1000);
    return false;
  }
}

bool sethostname(String uid) {
  Config_SelectSDCard();
  String filepath = "settings/hostname";
  SD.remove(filepath);
  File pwFile = SD.open(filepath, FILE_WRITE);

  if (pwFile) {
    pwFile.print(uid);
    pwFile.flush();
    pwFile.close();
    Config_DeselectAll();
    Serial.println("Hostname set: " + uid);
    Display_Show(String(" ") + APP_NAME, ">SET ADMIN: YES!");
    Buzzer_Play(1, 900, 100); delay(500);
    return true;
  } else {
    Config_DeselectAll();
    Serial.println("Failed to set hostname");
    Display_Show(String(" ") + APP_NAME, ">SET ADMIN: NO!");
    Buzzer_Play(1, 300, 100); delay(500);
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
  } else if(Rfid_Status() == "sethostname") {
    return sethostname(uid);
  }  else {
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
  Config_AddBootLog("System initialized");
  
  Relay_Init();
  Config_AddBootLog("Relay initialized");
  Buzzer_Play(1, 500, 200);

  Display_Init();
  Display_Show(String(" ") + APP_NAME, "by BytesCrafter");
  Buzzer_Play(1, 200, 1000);

  SDCard_Init(&Display_Show);
  Buzzer_Play(1, 200, 500);

  Rfid_Init(&Display_Show);
  Buzzer_Play(1, 200, 500);

  Hotspot_broadcast();
  Buzzer_Play(1, 200, 2500);
  
  Display_Show(String(" ") + APP_NAME, " TAP YOUR CARD");
  Buzzer_Play(2, 100, 100);

  Config_AddBootLog("Setup complete - Ready for operation");
  isLoaded = true;
}

bool didTryConnect = false;

void loop() {
  if(!isLoaded) {
    return ;
  }
  Config_Loop();

  Rfid_Listen(&catch_Rfid);

  Hotspot_loop();

  // if(!didTryConnect) {
  //   //WifiClient_connect();
  //   didTryConnect = true;
  // }

  // if (WiFi.status() == WL_CONNECTED) {
  //   timeClient.update();
  // }
}