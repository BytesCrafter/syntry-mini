/*
  Module: WiFi Client Module.
  Version: 0.1.0
*/

//Include library.
#include <WiFiClient.h>

//Connect your controller to WiFi
void WifiClient_connect(String apName = "", String apPass = "null") {

  //Get WiFi credentials from EEPROM
  String ssid = Config_LoadWifiSSID();
  String password = Config_LoadWifiPassword();
  
  // Use provided parameters if EEPROM is empty
  if(ssid == "" && apName != "") {
    ssid = apName;
  }
  if(password == "" && apPass != "null") {
    password = apPass;
  }
  
  // If still no credentials, return
  if(ssid == "") {
    Config_AddBootLog("WiFi: No credentials stored");
    return;
  }

  //Connect to WiFi Network
  Config_AddBootLog("WiFi: Connecting to " + ssid);
  Display_Show(String(" ") + APP_NAME, "WiFi Connecting.");
  Buzzer_Play(1, 900, 300);
  WiFi.begin(ssid, password);
  int retries = 0;
  
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {
    retries++;
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED || retries > 14) {
    Config_AddBootLog("WiFi: Connection failed");
    Display_Show(String(" ") + APP_NAME, "Wifi Conn Failed");
    Buzzer_Play(1, 900, 500);
    wifiStatus = false;
  } else {
    timeClient.begin();

    Config_AddBootLog("WiFi: Connected!");
    String ipAddress = WiFi.localIP().toString().c_str();
    Config_AddBootLog("WiFi: IP " + ipAddress);
    wifiStatus = true;

    String filepath = "settings/ipaddress";
    SD.remove(filepath); //,ake sure delete if existing.
    File ipadd = SD.open(filepath, FILE_WRITE);
    if (ipadd) {
      ipadd.print(ipAddress);
      ipadd.close();
    }

    Display_Show(String(" ") + APP_NAME, ">"+ipAddress);
    Buzzer_Play(1, 1200, 500);
    delay(1000);
  }
}