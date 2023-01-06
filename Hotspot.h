#include <DNSServer.h>
#include <ESP8266WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(172, 217, 28, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

String rfidMode = "access"; //access, add, remove

String Rfid_Status() {
  return rfidMode;
}

void Hotspot_broadcast() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Syntry Mini v1");

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  webServer.on("/menu", []() {
    webServer.send(200, "text/html", Helper_Hotspot_To_Menu());
  });

  webServer.on("/access", []() {
    Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    rfidMode = "access";
    webServer.send(200, "text/html", Helper_Hotspot_To_Access());
  });

  webServer.on("/add", []() {
    Display_Show(" Syntry Mini v1", ">TAP TO REGISTER");
    rfidMode = "add";
    webServer.send(200, "text/html", Helper_Hotspot_To_Add());
  });

  webServer.on("/remove", []() {
    Display_Show(" Syntry Mini v1", ">TAP TO REMOVE");
    rfidMode = "remove";
    webServer.send(200, "text/html", Helper_Hotspot_To_Remove());
  });

  webServer.on("/verify", []() {
    Display_Show(" Syntry Mini v1", ">TAP TO VERIFY");
    rfidMode = "verify";
    webServer.send(200, "text/html", Helper_Hotspot_To_Verify());
  });

  // replay to all requests with same HTML
  webServer.onNotFound([]() {
    webServer.send(200, "text/html", Helper_Hotspot_Login());
  });
  webServer.begin();
}

void Hotspot_loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
