/*
  Module: WiFi AP Module.
  Version: 0.1.0
*/

#include <DNSServer.h>
#include <ESP8266WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

unsigned long startTime;
unsigned long expireTime = 300000; //5min

void Hotspot_broadcast() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  String preName = String(APP_NAME) + " - ";
  String postName = Config_LoadHostname();
  String allName = preName+postName;
  WiFi.softAP(allName);

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  webServer.on("/login", []() {
    String uname = webServer.arg("uname");
    String pword = webServer.arg("pword");

    // Get password from EEPROM (defaults to "admin" if not set)
    String storedPassword = Config_LoadPassword();
    bool isAuth = false;

    if(uname == "admin" && pword == storedPassword) {
      isAuth = true;
    }

    if(isAuth) {
      webServer.sendHeader("Location", String("/menu"), true);
      Display_Show(String(" ") + APP_NAME, "WELCOME! ADMIN");
      Buzzer_Play(1, 1000, 50); delay(1000);
      Display_Show(String(" ") + APP_NAME, " TAP YOUR CARD");
    } else {
      webServer.sendHeader("Location", String("/?status=Invalid%20username%20or%20password!"), true);
      Display_Show(String(" ") + APP_NAME, "REPORTING STOP!");
      Buzzer_Play(1, 250, 50); delay(1000);
      Display_Show(String(" ") + APP_NAME, " TAP YOUR CARD");
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

  webServer.on("/change-hostname", []() {
    String status = webServer.arg("status");
    webServer.send(200, "text/html", Helper_Hotspot_ChangeHostname(status));
  });

  webServer.on("/update-password", []() {
    String newpass = webServer.arg("newpass");
    String confirmpass = webServer.arg("confirmpass");

    if(newpass == confirmpass && newpass.length() > 0) {
      // Save password to EEPROM
      if(Config_SavePassword(confirmpass)) {
        Display_Show(String(" ") + APP_NAME, "PASSWORD UPDATED");
        Buzzer_Play(1, 900, 1000);

        webServer.sendHeader("Location", String("/change-password?status=Updated%20your%20Password!"), true);
        webServer.send ( 302, "text/plain", "");
        return;
      }
    }

    webServer.sendHeader("Location", String("/change-password?status=Something%20not%20right!"), true);
    webServer.send ( 302, "text/plain", "");
  });

  webServer.on("/update-hostname", []() {
    String newhostname = webServer.arg("newhostname");

    if(newhostname.length() > 0 && newhostname.length() <= 20) {
      // Save hostname to EEPROM
      if(Config_SaveHostname(newhostname)) {
        Display_Show(String(" ") + APP_NAME, "HOSTNAME UPDATED");
        Buzzer_Play(1, 900, 1000);

        webServer.sendHeader("Location", String("/change-hostname?status=Updated%20hostname!%20Restart%20to%20apply."), true);
        webServer.send ( 302, "text/plain", "");
        return;
      }
    }

    webServer.sendHeader("Location", String("/change-hostname?status=Something%20not%20right!"), true);
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

      Display_Show(String(" ") + APP_NAME, "Saved WIFI Creds");
      Buzzer_Play(1, 900, 500);

      Display_Show(String(" ") + APP_NAME, "Wifi Connecting.");
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
      Display_Show(String(" ") + APP_NAME, "ACCESS OVERRIDE");
      Buzzer_Play(1, 900, 50);
      Relay_Open();

      Display_Show(String(" ") + APP_NAME, " TAP YOUR CARD");
    } else {
      Display_Show(String(" ") + APP_NAME, " TAP YOUR CARD");
      rfidMode = "access";
      Buzzer_Play(1, 700, 50);
    }
    webServer.send(200, "text/html", Helper_Hotspot_To_Access());
  });

  webServer.on("/add", []() {
    startTime = millis();
    Display_Show(String(" ") + APP_NAME, ">TAP TO REGISTER");
    rfidMode = "add";
    Buzzer_Play(1, 700, 50);
    webServer.send(200, "text/html", Helper_Hotspot_To_Add());
  });

  webServer.on("/remove", []() {
    startTime = millis();
    Display_Show(String(" ") + APP_NAME, ">TAP TO REMOVE");
    rfidMode = "remove";
    Buzzer_Play(1, 700, 50);
    webServer.send(200, "text/html", Helper_Hotspot_To_Remove());
  });

  webServer.on("/verify", []() {
    startTime = millis();
    Display_Show(String(" ") + APP_NAME, ">TAP TO VERIFY");
    rfidMode = "verify";
    Buzzer_Play(1, 700, 50);
    webServer.send(200, "text/html", Helper_Hotspot_To_Verify());
  });

  webServer.on("/system", []() {
    webServer.send(200, "text/html", Helper_Hotspot_SystemInfo());
  });

  // replay to all requests with same HTML
  webServer.onNotFound([]() {
    String status = webServer.arg("status");
    webServer.send(200, "text/html", Helper_Hotspot_Login(status));
  });
  webServer.begin();
  wifiStatus = true;

  Display_Show(String(" ") + APP_NAME, "> WIFI Loaded...");
}

void Hotspot_loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();

  if(rfidMode != "access" && millis() - startTime > expireTime) {
    Display_Show(String(" ") + APP_NAME, " TAP YOUR CARD");
    rfidMode = "access";
    Buzzer_Play(1, 700, 50);
  }
}
