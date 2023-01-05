//Json add
#include <SPI.h>
#include <Wire.h> 
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "Settings.h"
#include "Config.h"
#include "Helper.h"
#include "Hotspot.h"
#include "WebRequest.h"
#include "WebServer.h"
#include "WifiClient.h"
#include "Display.h"
#include "Light.h"
#include "Rfid.h"
#include "Relay.h"
#include "SDCard.h"
//#include "Rtc.h"

struct settings {
  char ssid[50];
  char pword[50];
} user_wifi = {};

void setup() {
  //Initialized all the required config and libraries.
  Config_Init();
  Display_Init();
  Rfid_Init();
  Relay_Init();
  SDCard_Init();

  String data = SDCard_Load("test.txt");
  Serial.println(data);

  Rfid_Listen(&catch_Rfid); //TEMPORARY
  
  EEPROM.begin(sizeof(struct settings)); //Init localsave
  EEPROM.get(0, user_wifi);
  
  //Initial welcome display for the  system.
  Display_Show("Syntry Kiosk Mini v1", "Made by BytesCrafter", "  Getting Started.  ", "    Hello World!    ");

  //TODO: Check first if found a WifiClient config.
  // String ssidTest = SDCard_Load(100);
  // String pwordTest = SDCard_Load(200);

  // delay(100);

  // SDCard_Save(100, "GuestNetwork");
  // SDCard_Save(200, "ahm2022!");

  bool found = true; //TODO
  bool connected = true; //TODO

  //THIS IS TEMPORARY! TEST!
  WifiClient_connect("GuestNetwork", "ahm2022!");

  //Serve a local server for init wifi config.
  if(!found && !connected) {
    Hotspot_broadcast();

    webs.on("/", handle_OnConnect);
    webs.begin();
    return;
  }

  //delay(1500); //TEST
  //String savedSsid = "Syntry Test Domain"; //TODO

  //Display_Show("", "", "Connecting to WiFi..", savedSsid);
}

void handle_OnConnect() {
  Display_Show("", "", "Someone connected admin.", "Who are you!?");

  if(webs.arg("ssid") != "" && webs.arg("pw") != "") {

    // user_wifi.ssid = Helper_StringToChars(webs.arg("ssid"));
    // user_wifi.pword = Helper_StringToChars(webs.arg("pw"));
    // user_wifi.ssid[ webs.arg("ssid").length()] = user_wifi.pword[webs.arg("pw").length()] = 0;

    // strncpy(user_wifi.ssid. webs.arg("ssid").c_str(), sizeof(user_wifi.ssid));
    // strncpy(user_wifi.pword. webs.arg("pw").c_str(), sizeof(user_wifi.pword));
    // user_wifi.ssid[ webs.arg("ssid").length()] = user_wifi.pword[webs.arg("pw").length()] = 0;

    EEPROM.put(0, user_wifi);
    EEPROM.commit();

    WifiClient_connect(webs.arg("ssid"), webs.arg("pw"));

    if(WiFi.status() == WL_CONNECTED) {
      String savedSsid = "Syntry Test Domain"; //TODO
      Display_Show("", "", "Connected to Wifi AP called:", savedSsid);

      //TODO: Save to local prefs.

      webs.stop(); //auto
      WebServer_start();
    }
  }
  
  webs.send(200, "text/html", Helper_ConnectHtml()); 
}

void loop() {
  if(WiFi.status() != WL_CONNECTED) {
    Hotspot_loop();
    return;
  }

  //Serial.println( "Wifi Ready: " + Helper_WifiStatus(WiFi.status()) );
  WebServer_update();

  //CONDITION: WAIT FOR THE WIFI CONNECTED STATUS.
  Rfid_Listen(&catch_Rfid);

  //Display_Sample();

  //while (WiFi.status() == WL_CONNECTED) {
    //WebServer_update();
    //WebRequest_get(); //TODO, See Status
  //}

  // Light_Green(HIGH);
  // Relay_Off();
  // delay(500);
  // Light_Green(LOW);
  // Relay_On();
  // Helper_EmitTune(1, 900, 50);
}

bool catch_Rfid(String uid) {
  if(uid == "null") {
    return false;
  }

  Helper_EmitTune(1, 900, 50);

  Display_Show("Authenticating...", "UID: " + uid);
  Serial.println("UID: " + uid);

  //TODO: Web Request to Check if RFID is auth.
  delay(1000);
  String compare = "45C1572A";

  if(uid != compare) {//User not known.
    Display_Show("Syntry Report", "Access forbidden!");
    Serial.println("Access forbidden!");
    return false;
  }
  
  Display_Show("Welcome! Username", "Access granted!");
  Serial.println("Access granted!");
  Relay_Off();

  Helper_EmitTune(5, 1000, 50);
  delay(500); //5 SECONDS
  Relay_On();
  return true;
}