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

unsigned long startTime;
unsigned long expireTime = 300000; //5min

File adminFile;

void Hotspot_broadcast() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  String preName = "Syntry Mini - ";
  String postName = RST_KEYTAG;
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
    adminFile = SD.open(filepath);
    bool isAuth = false;

    if (adminFile) {
      String sdPass = adminFile.readString();
      if(uname == "admin" && pword == sdPass) {
        isAuth = true;
      }
    } else {
      if(uname == "admin" && pword == "admin") {
        isAuth = true;
      }
    }

    if(isAuth) {
      webServer.sendHeader("Location", String("/menu"), true);
      Display_Show(" Syntry Mini v1", "WELCOME! ADMIN");
      Buzzer_Play(1, 1000, 50);
    } else {
      webServer.sendHeader("Location", String("/"), true);
      Display_Show(" Syntry Mini v1", "REPORTING STOP!");
      Buzzer_Play(1, 250, 50);
      delay(1500);
      Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    }
    webServer.send ( 302, "text/plain", "");
  });

  webServer.on("/menu", []() {
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

      adminFile = SD.open(filepath, FILE_WRITE);

      if (adminFile) {
        adminFile.print(confirmpass);
        adminFile.close();
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

  webServer.on("/access", []() {
    Display_Show(" Syntry Mini v1", " TAP YOUR CARD");
    rfidMode = "access";
    Buzzer_Play(1, 700, 50);
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
