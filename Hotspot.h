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

// Session management
String activeSessionToken = "";
unsigned long sessionStartTime = 0;
const unsigned long SESSION_TIMEOUT = 1800000; // 30 minutes in milliseconds

// Generate random session token
String Hotspot_GenerateSessionToken() {
  String token = "";
  for(int i = 0; i < 32; i++) {
    token += String(random(0, 16), HEX);
  }
  return token;
}

// Check if session is valid
bool Hotspot_IsSessionValid() {
  if(activeSessionToken == "") {
    Serial.println("Session: No active token");
    return false;
  }
  
  // Check session timeout (30 minutes)
  if(millis() - sessionStartTime > SESSION_TIMEOUT) {
    activeSessionToken = ""; // Clear expired session
    Serial.println("Session: Expired");
    return false;
  }
  
  // Get session cookie from request
  if(!webServer.hasHeader("Cookie")) {
    Serial.println("Session: No cookie header");
    return false;
  }
  
  String cookieHeader = webServer.header("Cookie");
  String expectedCookie = "session=" + activeSessionToken;
  
  Serial.println("Session token: " + activeSessionToken);
  Serial.println("Cookie header: " + cookieHeader);
  
  bool isValid = cookieHeader.indexOf(expectedCookie) >= 0;
  Serial.println("Session valid: " + String(isValid ? "YES" : "NO"));
  
  return isValid;
}

// Redirect to login if not authenticated
void Hotspot_RequireAuth() {
  if(!Hotspot_IsSessionValid()) {
    webServer.sendHeader("Location", String("/?status=Please%20login%20first!"), true);
    webServer.send(200, "text/plain", "");
  }
}

void Hotspot_broadcast() {
  // Initialize random seed for session token generation
  randomSeed(micros());
  
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
    
    Serial.println("Login attempt - Username: " + uname);
    Serial.println("Stored password: " + storedPassword);
    Serial.println("Provided password: " + pword);

    if(uname == "admin" && pword == storedPassword) {
      // Create new session BEFORE any display operations
      activeSessionToken = Hotspot_GenerateSessionToken();
      sessionStartTime = millis();
      
      Serial.println("Login successful - Session token: " + activeSessionToken);
      Config_AddBootLog("Web: Admin logged in");
      
      // Set session cookie and send menu page directly (avoid redirect timing issue)
      String cookieValue = "session=" + activeSessionToken + "; Path=/; HttpOnly; Max-Age=1800";
      webServer.sendHeader("Set-Cookie", cookieValue);
      webServer.send(200, "text/html", Helper_Hotspot_To_Menu());
      
      // Update display AFTER sending response
      Display_Show(String(" ") + APP_NAME, "WELCOME! ADMIN");
      Buzzer_Play(1, 1000, 50);
      delay(500);
      Display_Show(String(" ") + APP_NAME, " TAP YOUR CARD");
    } else {
      Serial.println("Login failed - Invalid credentials");
      Config_AddBootLog("Web: Failed login attempt");
      
      // Redirect to login with error
      webServer.sendHeader("Location", "/?status=Invalid%20username%20or%20password!");
      webServer.send(200, "text/plain", "");
      
      // Update display AFTER sending response
      Display_Show(String(" ") + APP_NAME, "REPORTING STOP!");
      Buzzer_Play(1, 250, 50);
      delay(500);
      Display_Show(String(" ") + APP_NAME, " TAP YOUR CARD");
    }
  });

  webServer.on("/menu", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    
    String action = webServer.arg("action");
    if(action == "restart") {
      ESP.reset();
    }
    webServer.send(200, "text/html", Helper_Hotspot_To_Menu());
  });

  webServer.on("/change-password", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    String status = webServer.arg("status");
    webServer.send(200, "text/html", Helper_Hotspot_ChangePassword(status));
  });

  webServer.on("/change-hostname", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    String status = webServer.arg("status");
    webServer.send(200, "text/html", Helper_Hotspot_ChangeHostname(status));
  });

  webServer.on("/manage-users", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    String status = webServer.arg("status");
    webServer.send(200, "text/html", Helper_Hotspot_ManageUsers(status));
  });

  webServer.on("/boot-logs", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    webServer.send(200, "text/html", Helper_Hotspot_BootLogs());
  });

  webServer.on("/delete-user", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    
    String uid = webServer.arg("uid");

    if(uid.length() > 0) {
      Config_SelectSDCard();
      String filepath = "users/" + uid;
      bool removed = SD.remove(filepath);
      Config_DeselectAll();
      
      if(removed) {
        Display_Show(String(" ") + APP_NAME, "USER DELETED");
        Buzzer_Play(1, 900, 500);
        Serial.println("User deleted via web: " + uid);
        
        webServer.sendHeader("Location", String("/manage-users?status=User%20deleted!"), true);
        webServer.send(200, "text/plain", "");
        return;
      }
    }

    webServer.sendHeader("Location", String("/manage-users?status=Failed%20to%20delete%20user!"), true);
    webServer.send(200, "text/plain", "");
  });

  webServer.on("/update-password", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    
    String newpass = webServer.arg("newpass");
    String confirmpass = webServer.arg("confirmpass");

    if(newpass == confirmpass && newpass.length() > 0) {
      // Save password to EEPROM
      if(Config_SavePassword(confirmpass)) {
        Display_Show(String(" ") + APP_NAME, "PASSWORD UPDATED");
        Buzzer_Play(1, 900, 1000);

        webServer.sendHeader("Location", String("/change-password?status=Updated%20your%20Password!"), true);
        webServer.send ( 200, "text/plain", "");
        return;
      }
    }

    webServer.sendHeader("Location", String("/change-password?status=Something%20not%20right!"), true);
    webServer.send ( 200, "text/plain", "");
  });

  webServer.on("/update-hostname", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    
    String newhostname = webServer.arg("newhostname");

    if(newhostname.length() > 0 && newhostname.length() <= 20) {
      // Save hostname to EEPROM
      if(Config_SaveHostname(newhostname)) {
        Display_Show(String(" ") + APP_NAME, "HOSTNAME UPDATED");
        Buzzer_Play(1, 900, 1000);

        webServer.sendHeader("Location", String("/change-hostname?status=Updated%20hostname!%20Restart%20to%20apply."), true);
        webServer.send ( 200, "text/plain", "");
        return;
      }
    }

    webServer.sendHeader("Location", String("/change-hostname?status=Something%20not%20right!"), true);
    webServer.send ( 200, "text/plain", "");
  });

  webServer.on("/wifi-connect", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    String status = webServer.arg("status");
    webServer.send(200, "text/html", Helper_Hotspot_ConnectWifi(status));
  });

  webServer.on("/save-wifi", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    
    String wifiname = webServer.arg("wifiname");
    String wifipass = webServer.arg("wifipass");

    if(wifiname != "" && wifipass != "") {
      // Save to EEPROM
      bool ssidSaved = Config_SaveWifiSSID(wifiname);
      bool passSaved = Config_SaveWifiPassword(wifipass);
      
      if(ssidSaved && passSaved) {
        Config_AddBootLog("WiFi: Credentials saved to EEPROM");
        Display_Show(String(" ") + APP_NAME, "Saved WIFI Creds");
        Buzzer_Play(1, 900, 500);

        Display_Show(String(" ") + APP_NAME, "Wifi Connecting.");
        Buzzer_Play(1, 900, 500);

        WifiClient_connect(wifiname, wifipass);
        timeClient.begin();

        webServer.sendHeader("Location", String("/wifi-connect?status=Saved%20and%20connected!"), true);
        webServer.send ( 200, "text/plain", "");
        return;
      } else {
        webServer.sendHeader("Location", String("/wifi-connect?status=Credentials%20too%20long!"), true);
        webServer.send ( 200, "text/plain", "");
        return;
      }
    }

    webServer.sendHeader("Location", String("/wifi-connect?status=Missing%20credentials!"), true);
    webServer.send ( 200, "text/plain", "");
  });

  webServer.on("/wifi-manual-connect", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    WifiClient_connect();
    webServer.sendHeader("Location", String("/wifi-connect?status=Connecting..."), true);
    webServer.send ( 200, "text/plain", "");
  });

  webServer.on("/wifi-disconnect", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    WiFi.disconnect();
    wifiStatus = false;
    Config_AddBootLog("WiFi: Manually disconnected");
    webServer.sendHeader("Location", String("/wifi-connect?status=Disconnected"), true);
    webServer.send ( 200, "text/plain", "");
  });

  webServer.on("/wifi-toggle-auto", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    bool currentAuto = Config_LoadWifiAuto();
    Config_SaveWifiAuto(!currentAuto);
    String status = !currentAuto ? "Auto-connect%20enabled" : "Auto-connect%20disabled";
    Config_AddBootLog("WiFi: Auto-connect " + String(!currentAuto ? "enabled" : "disabled"));
    webServer.sendHeader("Location", String("/wifi-connect?status=") + status, true);
    webServer.send ( 200, "text/plain", "");
  });

  webServer.on("/access", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    
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
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    startTime = millis();
    Display_Show(String(" ") + APP_NAME, ">TAP TO REGISTER");
    rfidMode = "add";
    Buzzer_Play(1, 700, 50);
    webServer.send(200, "text/html", Helper_Hotspot_To_Add());
  });

  webServer.on("/remove", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    startTime = millis();
    Display_Show(String(" ") + APP_NAME, ">TAP TO REMOVE");
    rfidMode = "remove";
    Buzzer_Play(1, 700, 50);
    webServer.send(200, "text/html", Helper_Hotspot_To_Remove());
  });

  webServer.on("/verify", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    startTime = millis();
    Display_Show(String(" ") + APP_NAME, ">TAP TO VERIFY");
    rfidMode = "verify";
    Buzzer_Play(1, 700, 50);
    webServer.send(200, "text/html", Helper_Hotspot_To_Verify());
  });

  webServer.on("/system", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    webServer.send(200, "text/html", Helper_Hotspot_SystemInfo());
  });

  // Logout route - clear session
  webServer.on("/logout", []() {
    activeSessionToken = ""; // Clear session
    sessionStartTime = 0;
    webServer.sendHeader("Set-Cookie", "session=; Path=/; Max-Age=0"); // Clear cookie
    webServer.sendHeader("Location", String("/?status=Logged%20out%20successfully!"), true);
    webServer.send(200, "text/plain", "");
    Config_AddBootLog("Web: Admin logged out");
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
