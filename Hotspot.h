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

  webServer.on("/login", []() {
    String uname = webServer.arg("uname");
    String pword = webServer.arg("pword");

    if(uname == "admin" && pword == "bytescrafter2023!") {
      webServer.sendHeader("Location", String("/menu"), true);
      Display_Show(" Syntry Mini v1", "WELCOME! ADMIN");
       Buzzer_Play(1, 1000, 50);
    } else {
      webServer.sendHeader("Location", String("/"), true);
      Display_Show(" Syntry Mini v1", "STOP! REPORTING");
      Buzzer_Play(1, 250, 50);
      delay(1500);
      Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    }
    webServer.send ( 302, "text/plain", "");
  });

  webServer.on("/menu", []() {
    webServer.send(200, "text/html", Helper_Hotspot_To_Menu());
  });

  webServer.on("/access", []() {
    Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    rfidMode = "access";
    Buzzer_Play(1, 700, 50);
    webServer.send(200, "text/html", Helper_Hotspot_To_Access());
  });

  webServer.on("/add", []() {
    Display_Show(" Syntry Mini v1", ">TAP TO REGISTER");
    rfidMode = "add";
    Buzzer_Play(1, 700, 50);
    webServer.send(200, "text/html", Helper_Hotspot_To_Add());
  });

  webServer.on("/remove", []() {
    Display_Show(" Syntry Mini v1", ">TAP TO REMOVE");
    rfidMode = "remove";
    Buzzer_Play(1, 700, 50);
    webServer.send(200, "text/html", Helper_Hotspot_To_Remove());
  });

  webServer.on("/verify", []() {
    Display_Show(" Syntry Mini v1", ">TAP TO VERIFY");
    rfidMode = "verify";
    Buzzer_Play(1, 700, 50);
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
