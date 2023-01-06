#include <WiFiClient.h>

/*
* Connect your controller to WiFi
*/
void WifiClient_connect(String ssid = "Syntry-AP", String password = "bytescrafter") {

  //Connect to WiFi Network
   Serial.println("Connecting to WiFi...");
   WiFi.begin(ssid, password);
   int retries = 0;

  //Make new line.
   Serial.println();
  
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {
    retries++;
    delay(500);
    Serial.print(".");
  }

  //Make new line.
   Serial.println();

  if (retries > 14) {
    Serial.println(F("WiFi connection FAILED"));
  }

  if (WiFi.status() == WL_CONNECTED) {
      Serial.println(F("WiFi CONNECTED!"));
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
  }
}