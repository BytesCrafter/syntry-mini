/*
  Module: WiFi AP Module.
  Version: 0.1.0
*/

#include <DNSServer.h>
#include <ESP8266WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(172, 217, 28, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

unsigned long startTime;
unsigned long expireTime = 300000; //5min

String Hotspot_Hostname() {
  String filepath = "settings/hostname";
  File curHost = SD.open(filepath);

  if (curHost) {
    return curHost.readString();
    curHost.close();
  } else {
    return Helper_RandomString();
  }
}

void Hotspot_broadcast() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  String preName = "Syntry Mini - ";
  String postName = Hotspot_Hostname();
  String allName = preName+postName;
  WiFi.softAP(allName);

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  webServer.on("/login", []() {
    String uname = webServer.arg("uname");
    String pword = webServer.arg("pword");

    //Check if SDCard has admin password.
    String filepath = "users/admin";
    File loginFile = SD.open(filepath);
    bool isAuth = false;

    if (loginFile) {
      String sdPass = loginFile.readString();
      if(uname == "admin" && pword == sdPass) {
        isAuth = true;
      }
      loginFile.close();
    } else {
      if(uname == "admin" && pword == "admin") {
        isAuth = true;
      }
    }

    if(isAuth) {
      webServer.sendHeader("Location", String("/menu"), true);
      Display_Show(" Syntry Mini v1", "WELCOME! ADMIN");
      Buzzer_Play(1, 1000, 50); delay(1000);
      Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    } else {
      webServer.sendHeader("Location", String("/"), true);
      Display_Show(" Syntry Mini v1", "REPORTING STOP!");
      Buzzer_Play(1, 250, 50); delay(1000);
      Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    }
    webServer.send ( 302, "text/plain", "");
  });

  webServer.on("/menu", []() {
    String action = webServer.arg("action");
    if(action == "restart") {
      ESP.reset();
    }
    webServer.send(200, "text/html", Helper_Hotspot_To_Menu());
  });

  webServer.on("/change-password", []() {
    String status = webServer.arg("status");
    webServer.send(200, "text/html", Helper_Hotspot_ChangePassword(status));
  });

  webServer.on("/update-password", []() {
    String newpass = webServer.arg("newpass");
    String confirmpass = webServer.arg("confirmpass");

    if(newpass == confirmpass) {
      String filepath = "users/admin";
      SD.remove(filepath); //,ake sure delete if existing.

      File pwFile = SD.open(filepath, FILE_WRITE);

      if (pwFile) {
        pwFile.print(confirmpass);
        pwFile.close();
        Display_Show(" Syntry Mini v1", "PASSWORD UPDATED");
        Buzzer_Play(1, 900, 1000);

        webServer.sendHeader("Location", String("/change-password?status=Updated%20your%20Password!"), true);
        webServer.send ( 302, "text/plain", "");
        return;
      }
    }

    webServer.sendHeader("Location", String("/change-password?status=Something%20not%20right!"), true);
    webServer.send ( 302, "text/plain", "");
  });

  webServer.on("/wifi-connect", []() {
    String status = webServer.arg("status");
    webServer.send(200, "text/html", Helper_Hotspot_ConnectWifi(status));
  });

  webServer.on("/save-wifi", []() {
    String wifiname = webServer.arg("wifiname");
    String wifipass = webServer.arg("wifipass");

    if(wifiname != "" && wifipass != "") {
      String filepath;
      File wifiCon;      

      filepath = "settings/wifiname";
      SD.remove(filepath); //,ake sure delete if existing.
      wifiCon = SD.open(filepath, FILE_WRITE);
      if (wifiCon) {
        wifiCon.print(wifiname);
        wifiCon.close();
      }

      filepath = "settings/wifipass";
      SD.remove(filepath); //,ake sure delete if existing.
      wifiCon = SD.open(filepath, FILE_WRITE);
      if (wifiCon) {
        wifiCon.print(wifipass);
        wifiCon.close();
      }

      Display_Show(" Syntry Mini v1", "Saved WIFI Creds");
      Buzzer_Play(1, 900, 500);

      Display_Show(" Syntry Mini v1", "Wifi Connecting.");
      Buzzer_Play(1, 900, 500);

      WifiClient_connect(wifiname, wifipass);
      timeClient.begin();

      webServer.sendHeader("Location", String("/wifi-connect?status=Saved%20wifi%20Credential!"), true);
      webServer.send ( 302, "text/plain", "");
      return;
    }

    webServer.sendHeader("Location", String("/wifi-connect?status=Something%20not%20right!"), true);
    webServer.send ( 302, "text/plain", "");
  });

  webServer.on("/access", []() {
    String action = webServer.arg("action");

    if(action == "override") {
      //TODO: Save log to SDCard
      //SDCard_Save("logs.txt", "User and Time Here"); //sHUTDOWN
      Display_Show(" Syntry Mini v1", "ACCESS OVERRIDE");
      Buzzer_Play(1, 900, 50);
      Relay_Open();

      Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    } else {
      Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
      rfidMode = "access";
      Buzzer_Play(1, 700, 50);
    }
    webServer.send(200, "text/html", Helper_Hotspot_To_Access());
  });

  webServer.on("/add", []() {
    startTime = millis();
    Display_Show(" Syntry Mini v1", ">TAP TO REGISTER");
    rfidMode = "add";
    Buzzer_Play(1, 700, 50);
    webServer.send(200, "text/html", Helper_Hotspot_To_Add());
  });

  webServer.on("/remove", []() {
    startTime = millis();
    Display_Show(" Syntry Mini v1", ">TAP TO REMOVE");
    rfidMode = "remove";
    Buzzer_Play(1, 700, 50);
    webServer.send(200, "text/html", Helper_Hotspot_To_Remove());
  });

  webServer.on("/verify", []() {
    startTime = millis();
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
  if(rfidMode != "access" && millis() - startTime > expireTime) {
    Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    rfidMode = "access";
    Buzzer_Play(1, 700, 50);
  }
  
  dnsServer.processNextRequest();
  webServer.handleClient();
}
