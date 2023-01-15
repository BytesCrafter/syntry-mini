/*
  Module: WiFi Client Module.
  Version: 0.1.0
*/

//Include library.
#include <WiFiClient.h>

//Connect your controller to WiFi
void WifiClient_connect(String apName = "Syntry-AP", String apPass = "bytescrafter") {

  //Get WiFi Saved AP name.
  String ssid = apName;
  String wnpath = "settings/wifiname";
  File wnfile = SD.open(wnpath);
  if (wnfile) {
    ssid = wnfile.readString();
  }

  //Get WiFi Saved AP pword.
  String password = apPass;
  String wppath = "settings/wifipass";
  File wpfile = SD.open(wppath);
  if (wpfile) {
    password = wpfile.readString();
  }

  //Connect to WiFi Network
  Serial.println("Connecting to WiFi...");
  Display_Show(" Syntry Mini v1", "WiFi Connecting.");
  Buzzer_Play(1, 900, 300);
  WiFi.begin(ssid, password);
  int retries = 0;
  
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {
    retries++;
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED || retries > 14) {
    Serial.println("WiFi connection FAILED");
    Display_Show(" Syntry Mini v1", "Wifi Conn Failed");
    Buzzer_Play(1, 900, 500);
  } else {
    timeClient.begin();

    Serial.println("WiFi Connected!");
    String ipAddress = WiFi.localIP().toString().c_str();
    Serial.println("IP address: " + ipAddress);

    String filepath = "settings/ipaddress";
    SD.remove(filepath); //,ake sure delete if existing.
    File ipadd = SD.open(filepath, FILE_WRITE);
    if (ipadd) {
      ipadd.print(ipAddress);
      ipadd.close();
    }

    Display_Show(" Syntry Mini v1", ">"+ipAddress);
    Buzzer_Play(1, 1200, 500);
    delay(1000);
  }
}