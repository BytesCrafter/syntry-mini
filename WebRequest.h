//STATUS: PENDING
//NOTE: Http need yto be https
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

void WebRequest_get() {
  WiFiClient client;
  HTTPClient http;

  String serverPath = "https://httpbin.org/get";
  
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverPath.c_str());

  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
    
  // Send HTTP GET request
  int httpResponseCode = http.GET();

  Serial.print(serverPath.c_str());
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  
  // Free resources
  http.end();
}