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
    return false;
  }
  
  // Check session timeout (30 minutes)
  if(millis() - sessionStartTime > SESSION_TIMEOUT) {
    activeSessionToken = ""; // Clear expired session
    Serial.println("Session: Expired");
    return false;
  }
  
  // Check session token in URL parameter (captive portal compatible)
  String token = webServer.arg("token");
  
  return token == activeSessionToken;
}

// Redirect to login if not authenticated
void Hotspot_RequireAuth() {
  if(!Hotspot_IsSessionValid()) {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1.0'>";
    html += "<meta http-equiv='refresh' content='0; url=/?status=Please%20login%20first!'>";
    html += "<style>body{margin:0;font-family:Arial,sans-serif;display:flex;align-items:center;justify-content:center;min-height:100vh;background:linear-gradient(135deg,#1ab394 0%,#138d75 100%);}";
    html += ".msg{text-align:center;color:#fff;}.msg h1{font-size:24px;margin:0 0 10px 0;font-weight:300;}.msg p{font-size:14px;opacity:0.9;margin:0;}</style>";
    html += "</head><body><div class='msg'><h1>" + String(APP_NAME) + "</h1><p>Redirecting to login...</p></div></body></html>";
    webServer.send(200, "text/html", html);
  }
}

// Get current session token parameter for URL generation
String Hotspot_GetTokenParam() {
  if(activeSessionToken != "") {
    return "?token=" + activeSessionToken;
  }
  return "";
}

// Add token to URL with proper separator
String Hotspot_AddToken(String url) {
  if(activeSessionToken != "") {
    if(url.indexOf('?') >= 0) {
      return url + "&token=" + activeSessionToken;
    } else {
      return url + "?token=" + activeSessionToken;
    }
  }
  return url;
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
      
      // Redirect to menu with token in URL (captive portal compatible)
      String html = "<!DOCTYPE html><html><head>";
      html += "<meta charset='UTF-8'>";
      html += "<meta name='viewport' content='width=device-width,initial-scale=1.0'>";
      html += "<meta http-equiv='refresh' content='0; url=/menu?token=" + activeSessionToken + "'>";
      html += "<style>body{margin:0;font-family:Arial,sans-serif;display:flex;align-items:center;justify-content:center;min-height:100vh;background:linear-gradient(135deg,#1ab394 0%,#138d75 100%);}";
      html += ".msg{text-align:center;color:#fff;}.msg h1{font-size:24px;margin:0 0 10px 0;font-weight:300;}.msg p{font-size:14px;opacity:0.9;margin:0;}</style>";
      html += "</head><body><div class='msg'><h1>" + String(APP_NAME) + "</h1><p>Login successful, redirecting...</p></div></body></html>";
      webServer.send(200, "text/html", html);
      
      // Update display AFTER sending response
      Display_Show(String(" ") + APP_NAME, "WELCOME! ADMIN");
      Buzzer_Play(1, 1000, 50);
      delay(500);
      Display_Show(String(" ") + APP_NAME, " TAP YOUR CARD");
    } else {
      Serial.println("Login failed - Invalid credentials");
      Config_AddBootLog("Web: Failed login attempt");
      
      // Send login page with error message (avoid redirect timing issue)
      webServer.send(200, "text/html", Helper_Hotspot_Login("Invalid username or password!"));
      
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
    int page = webServer.arg("page").toInt();  // Get page number (default 0)
    webServer.send(200, "text/html", Helper_Hotspot_ManageUsers(status, page));
  });

  webServer.on("/boot-logs", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    webServer.send(200, "text/html", Helper_Hotspot_BootLogs());
  });

  webServer.on("/edit-user", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    
    String uid = webServer.arg("uid");
    String status = webServer.arg("status");
    int page = webServer.arg("page").toInt();
    
    // Read current nickname from file
    String nickname = "";
    if(uid.length() > 0) {
      Config_SelectSDCard();
      File f = SD.open("users/" + uid, FILE_READ);
      if(f) {
        if(f.available()) {
          nickname = f.readStringUntil('\n');
          nickname.trim();
        }
        f.close();
      }
      Config_DeselectAll();
    }
    
    webServer.send(200, "text/html", Helper_Hotspot_EditUser(uid, nickname, status, page));
  });

  webServer.on("/add-user", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    String status = webServer.arg("status");
    webServer.send(200, "text/html", Helper_Hotspot_AddUser(status));
  });

  webServer.on("/create-user", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    
    String uid = webServer.arg("uid");
    String nickname = webServer.arg("nickname");
    
    // Validate UID (must be hex chars, 4-16 length)
    uid.toUpperCase();
    bool validUid = uid.length() >= 4 && uid.length() <= 16;
    for(int i = 0; i < uid.length() && validUid; i++) {
      char c = uid.charAt(i);
      if(!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) validUid = false;
    }
    
    if(!validUid) {
      webServer.sendHeader("Location", Hotspot_AddToken("/add-user?status=Invalid%20UID%20format!"), true);
      webServer.send(302, "text/plain", "");
      return;
    }
    
    // Check if user already exists
    Config_SelectSDCard();
    String filepath = "users/" + uid;
    if(SD.exists(filepath)) {
      Config_DeselectAll();
      webServer.sendHeader("Location", Hotspot_AddToken("/add-user?status=User%20already%20exists!"), true);
      webServer.send(302, "text/plain", "");
      return;
    }
    
    // Create new user file
    File f = SD.open(filepath, FILE_WRITE);
    if(f) {
      if(nickname.length() > 0) {
        f.print(nickname);
      }
      f.close();
      Config_DeselectAll();
      
      Display_Show(String(" ") + APP_NAME, "USER ADDED");
      Buzzer_Play(1, 900, 500);
      Serial.println("User added via web: " + uid + " (" + nickname + ")");
      
      webServer.sendHeader("Location", Hotspot_AddToken("/manage-users?status=User%20" + uid + "%20added!"), true);
      webServer.send(302, "text/plain", "");
      return;
    }
    Config_DeselectAll();
    
    webServer.sendHeader("Location", Hotspot_AddToken("/add-user?status=Failed%20to%20create%20user!"), true);
    webServer.send(302, "text/plain", "");
  });

  webServer.on("/save-user", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    
    String uid = webServer.arg("uid");
    String nickname = webServer.arg("nickname");
    int page = webServer.arg("page").toInt();
    
    if(uid.length() > 0) {
      Config_SelectSDCard();
      // Remove old file and create new with nickname content
      String filepath = "users/" + uid;
      SD.remove(filepath);
      File f = SD.open(filepath, FILE_WRITE);
      if(f) {
        if(nickname.length() > 0) {
          f.print(nickname);
        }
        f.close();
        Config_DeselectAll();
        
        Serial.println("User nickname saved: " + uid + " -> " + nickname);
        webServer.sendHeader("Location", Hotspot_AddToken("/manage-users?status=Nickname%20saved!&page=" + String(page)), true);
        webServer.send(302, "text/plain", "");
        return;
      }
      Config_DeselectAll();
    }
    
    webServer.sendHeader("Location", Hotspot_AddToken("/edit-user?uid=" + uid + "&status=Failed%20to%20save!&page=" + String(page)), true);
    webServer.send(302, "text/plain", "");
  });

  webServer.on("/delete-user", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    
    String uid = webServer.arg("uid");
    String pageStr = webServer.arg("page");
    int page = pageStr.toInt();

    if(uid.length() > 0) {
      Config_SelectSDCard();
      String filepath = "users/" + uid;
      bool removed = SD.remove(filepath);
      Config_DeselectAll();
      
      if(removed) {
        Display_Show(String(" ") + APP_NAME, "USER DELETED");
        Buzzer_Play(1, 900, 500);
        Serial.println("User deleted via web: " + uid);
        
        webServer.sendHeader("Location", Hotspot_AddToken("/manage-users?status=User%20deleted!&page=" + String(page)), true);
        webServer.send(302, "text/plain", "");
        return;
      }
    }

    webServer.sendHeader("Location", Hotspot_AddToken("/manage-users?status=Failed%20to%20delete%20user!&page=" + String(page)), true);
    webServer.send(302, "text/plain", "");
  });

  webServer.on("/clear-all-users", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    
    String adminpass = webServer.arg("adminpass");
    String storedPassword = Config_LoadPassword();
    
    // Verify admin password
    if(adminpass != storedPassword) {
      webServer.sendHeader("Location", Hotspot_AddToken("/manage-users?status=Invalid%20password!"), true);
      webServer.send(302, "text/plain", "");
      return;
    }
    
    // Delete all user files
    Config_SelectSDCard();
    File usersDir = SD.open("/users");
    int deletedCount = 0;
    
    if(usersDir && usersDir.isDirectory()) {
      File entry = usersDir.openNextFile();
      
      // Collect filenames first (can't delete while iterating)
      String filesToDelete[100];
      int fileCount = 0;
      
      while(entry && fileCount < 100) {
        if(!entry.isDirectory()) {
          String fileName = String(entry.name());
          int lastSlash = fileName.lastIndexOf('/');
          if(lastSlash >= 0) {
            fileName = fileName.substring(lastSlash + 1);
          }
          if(fileName != "admin" && fileName.length() > 0) {
            filesToDelete[fileCount++] = fileName;
          }
        }
        entry.close();
        entry = usersDir.openNextFile();
      }
      usersDir.close();
      
      // Now delete all collected files
      for(int i = 0; i < fileCount; i++) {
        String filepath = "users/" + filesToDelete[i];
        if(SD.remove(filepath)) {
          deletedCount++;
        }
      }
    }
    Config_DeselectAll();
    
    Display_Show(String(" ") + APP_NAME, "CLEARED " + String(deletedCount) + " USERS");
    Buzzer_Play(2, 900, 300);
    Serial.println("Cleared all users via web: " + String(deletedCount) + " deleted");
    Config_AddBootLog("Web: Cleared " + String(deletedCount) + " users");
    
    webServer.sendHeader("Location", Hotspot_AddToken("/manage-users?status=Cleared%20" + String(deletedCount) + "%20users!"), true);
    webServer.send(302, "text/plain", "");
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

        webServer.sendHeader("Location", Hotspot_AddToken("/change-password?status=Updated%20your%20Password!"), true);
        webServer.send(302, "text/plain", "");
        return;
      }
    }

    webServer.sendHeader("Location", Hotspot_AddToken("/change-password?status=Something%20not%20right!"), true);
    webServer.send(302, "text/plain", "");
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

        webServer.sendHeader("Location", Hotspot_AddToken("/change-hostname?status=Updated%20hostname!%20Restart%20to%20apply."), true);
        webServer.send(302, "text/plain", "");
        return;
      }
    }

    webServer.sendHeader("Location", Hotspot_AddToken("/change-hostname?status=Something%20not%20right!"), true);
    webServer.send(302, "text/plain", "");
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

    if(wifiname != "") {
      // Save to EEPROM (password can be empty for open networks)
      bool ssidSaved = Config_SaveWifiSSID(wifiname);
      bool passSaved = Config_SaveWifiPassword(wifipass);
      
      if(ssidSaved) {
        Config_AddBootLog("WiFi: Credentials saved to EEPROM");
        Display_Show(String(" ") + APP_NAME, "Saved WIFI Creds");
        Buzzer_Play(1, 900, 500);

        Display_Show(String(" ") + APP_NAME, "Wifi Connecting.");
        Buzzer_Play(1, 900, 500);

        WifiClient_connect(wifiname, wifipass);
        timeClient.begin();

        webServer.sendHeader("Location", Hotspot_AddToken("/wifi-connect?status=Saved%20and%20connected!"), true);
        webServer.send(302, "text/plain", "");
        return;
      } else {
        webServer.sendHeader("Location", Hotspot_AddToken("/wifi-connect?status=Credentials%20too%20long!"), true);
        webServer.send(302, "text/plain", "");
        return;
      }
    }

    webServer.sendHeader("Location", Hotspot_AddToken("/wifi-connect?status=Missing%20credentials!"), true);
    webServer.send(302, "text/plain", "");
  });

  webServer.on("/wifi-manual-connect", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    WifiClient_connect();
    webServer.sendHeader("Location", Hotspot_AddToken("/wifi-connect?status=Connecting..."), true);
    webServer.send(302, "text/plain", "");
  });

  webServer.on("/wifi-disconnect", []() {
    if(!Hotspot_IsSessionValid()) {
      Hotspot_RequireAuth();
      return;
    }
    WiFi.disconnect();
    wifiStatus = false;
    Config_AddBootLog("WiFi: Manually disconnected");
    webServer.sendHeader("Location", Hotspot_AddToken("/wifi-connect?status=Disconnected"), true);
    webServer.send(302, "text/plain", "");
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
    webServer.sendHeader("Location", Hotspot_AddToken("/wifi-connect?status=" + status), true);
    webServer.send(302, "text/plain", "");
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
    webServer.sendHeader("Location", Hotspot_AddToken("/?status=Logged%20out%20successfully!"), true);
    webServer.send(302, "text/plain", "");
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
