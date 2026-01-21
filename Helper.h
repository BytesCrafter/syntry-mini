/*
  Module: Helper Script.
  Version: 0.1.0
*/

String Helper_RandomString() {
  const int len = 10;
  char lists[] = {'1', '2','3', '4', '5','6', '7', '8', '9', '0'};
  const byte listLen = sizeof(lists) / sizeof(lists[0]);
  char notes[len+1];  //allow 1 extra for the NULL

  for (int n = 0; n < len ; n++) {
    notes[n] = lists[random(0, listLen)];
    notes[n + 1] = '\0';
  }

  return String(notes);
}

char* Helper_StringToChars(String str) {
  // Length (with one extra character for the null terminator)
  int str_len = str.length() + 1; 

  // Prepare the character array (the buffer) 
  char char_array[str_len];

  // Copy it over 
  str.toCharArray(char_array, str_len);

  return char_array;
}

void Helper_array_to_string(byte array[], unsigned int len, char buffer[]) {
   for (unsigned int i = 0; i < len; i++)
   {
      byte nib1 = (array[i] >> 4) & 0x0F;
      byte nib2 = (array[i] >> 0) & 0x0F;
      buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
      buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
   }
   buffer[len*2] = '\0';
}

// CSS stored in PROGMEM to save RAM (~900 bytes saved)
const char HTTP_STYLE[] PROGMEM = R"===(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Syntry</title><style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:Arial,sans-serif;background:linear-gradient(135deg,#1ab394 0%,#26d0b0 100%);min-height:100vh;display:flex;align-items:center;justify-content:center;padding:20px}
.c{background:#fff;border-radius:16px;box-shadow:0 10px 40px rgba(0,0,0,.15);padding:40px;max-width:500px;width:100%}
h1{color:#1a1a1a;font-size:28px;margin-bottom:8px;text-align:center;font-weight:700}
h5{color:#6c757d;font-size:14px;margin-bottom:30px;text-align:center;font-weight:400}
h6{color:#dc3545;font-size:13px;margin-bottom:15px;text-align:center}
input[type=text],input[type=password]{width:100%;padding:14px;border:2px solid #e9ecef;border-radius:12px;font-size:15px;margin-bottom:16px;transition:.3s;background:#f8f9fa}
input:focus{outline:none;border-color:#1ab394;background:#fff}
input[type=submit]{width:100%;padding:16px;background:#1ab394;color:#fff;border:none;border-radius:12px;font-size:16px;font-weight:600;cursor:pointer;transition:.3s;margin-bottom:12px;box-shadow:0 4px 15px rgba(26,179,148,.3)}
input[type=submit]:hover{background:#17a085;transform:translateY(-2px);box-shadow:0 6px 20px rgba(26,179,148,.4)}
input[type=submit]:active{transform:translateY(0)}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-bottom:20px}
.grid form{margin:0}
.grid input[type=submit]{height:67px;margin:0;font-size:15px;padding:10px}
.split{display:grid;grid-template-columns:1fr 1fr;gap:12px}
.split form{margin:0}
.split input[type=submit]{margin:0}
</style></head><body><div class="c">
)===";

String Helper_HttpHeader() {
  return FPSTR(HTTP_STYLE);
}

String Helper_HttpFooter() {
  String ptr = "</div></body></html>";
  return ptr;
}

String Helper_HttpBackToMenu() {
  extern String activeSessionToken;
  String ptr = F("<form action='/menu' method='get'><input type='hidden' name='token' value='");
  ptr += activeSessionToken;
  ptr += F("'><input type='submit' value='Back to Menu'></form>");
  return ptr;
}

String Helper_Hotspot_Login(String message = "") {
  String ptr = Helper_HttpHeader();
  ptr += String("<h1>") + APP_NAME + F("</h1><h5>Powered by ERPat System</h5>");
  if(message.length() > 0) { ptr += F("<h6>"); ptr += message; ptr += F("</h6>"); }
  ptr += F("<form action='/login' method='get'>");
  ptr += F("<input type='text' name='uname' placeholder='Username' required>");
  ptr += F("<input type='password' name='pword' placeholder='Password' required>");
  ptr += F("<input type='submit' value='LOGIN'>");
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_To_Menu() {
  extern String activeSessionToken;
  String token = "<input type='hidden' name='token' value='" + activeSessionToken + "'>";
  
  String ptr;
  ptr.reserve(2500);
  ptr = Helper_HttpHeader();
  ptr += String("<h1>") + APP_NAME + F("</h1><h5>Control Panel</h5><div class='grid'>");
  
  ptr += F("<form action='/verify' method='get'>");
  ptr += token;
  ptr += F("<input type='submit' value='Verify Mode'></form>");
  
  ptr += F("<form action='/access' method='get'>");
  ptr += token;
  ptr += F("<input type='submit' value='Access Mode'></form>");
  
  ptr += F("<form action='/add' method='get'>");
  ptr += token;
  ptr += F("<input type='submit' value='Add Card'></form>");
  
  ptr += F("<form action='/remove' method='get'>");
  ptr += token;
  ptr += F("<input type='submit' value='Remove Card'></form>");
  
  ptr += F("<form action='/change-password' method='get'>");
  ptr += token;
  ptr += F("<input type='submit' value='Change Password'></form>");
  
  ptr += F("<form action='/change-hostname' method='get'>");
  ptr += token;
  ptr += F("<input type='submit' value='Change Hostname'></form>");
  
  ptr += F("<form action='/boot-logs' method='get'>");
  ptr += token;
  ptr += F("<input type='submit' value='Boot Logs' style='background:#17a2b8'></form>");
  
  ptr += F("<form action='/wifi-connect' method='get'>");
  ptr += token;
  ptr += F("<input type='submit' value='WiFi Setup' style='background:#3498db'></form>");
  
  int userCount = SDCard_CountUsers();
  ptr += F("<form action='/manage-users' method='get'>");
  ptr += token;
  ptr += F("<input type='submit' value='Manage Users\n(");
  ptr += String(userCount);
  ptr += F(" cards)' style='background:#8e44ad;white-space:pre-wrap;line-height:1.3'></form>");
  
  ptr += F("<form action='/system' method='get'>");
  ptr += token;
  ptr += F("<input type='submit' value='System Info' style='background:#f39c12'></form></div><div class='split'>");
  
  ptr += F("<form action='/menu' method='get'>");
  ptr += token;
  ptr += F("<input type='hidden' name='action' value='restart'><input type='submit' value='Restart' style='background:#FFC107;color:#000'></form>");
  
  ptr += F("<form action='/logout' method='get'>");
  ptr += token;
  ptr += F("<input type='submit' value='Logout' style='background:#e53e3e'></form></div>");
  
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_To_Access() {
  extern String activeSessionToken;
  String token = "<input type='hidden' name='token' value='" + activeSessionToken + "'>";
  
  String ptr = Helper_HttpHeader();
  ptr += F("<h1>Access Mode</h1><h5>Tap your RFID card</h5><form action='/access' method='get'>");
  ptr += token;
  ptr += F("<input type='hidden' name='action' value='override'><input type='submit' value='Override Access' style='background:#48bb78'></form>");
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_To_Add() {
  String ptr = Helper_HttpHeader();
  ptr += F("<h1>Add Card Mode</h1><h5>Tap card to register</h5>");
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_To_Remove() {
  String ptr = Helper_HttpHeader();
  ptr += F("<h1>Remove Card Mode</h1><h5>Tap card to delete</h5>");
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_To_Verify() {
  String ptr = Helper_HttpHeader();
  ptr += F("<h1>Verify Mode</h1><h5>Tap card to check</h5>");
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_ChangePassword(String message = "") {
  extern String activeSessionToken;
  String ptr = Helper_HttpHeader();
  ptr += F("<h1>Change Password</h1><h5>Update admin credentials</h5>");
  if(message.length() > 0) { ptr += F("<h6>"); ptr += message; ptr += F("</h6>"); }
  ptr += F("<form action='/update-password' method='get'><input type='hidden' name='token' value='");
  ptr += activeSessionToken;
  ptr += F("'><input type='password' name='newpass' placeholder='New Password' required>");
  ptr += F("<input type='password' name='confirmpass' placeholder='Confirm Password' required>");
  ptr += F("<input type='submit' value='UPDATE'></form>");
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_ChangeHostname(String message = "") {
  extern String activeSessionToken;
  String currentHostname = Config_LoadHostname();
  
  String ptr = Helper_HttpHeader();
  ptr += F("<h1>Change Hostname</h1><h5>Update device name</h5>");
  if(message.length() > 0) { ptr += F("<h6>"); ptr += message; ptr += F("</h6>"); }
  if(currentHostname.length() > 0) { 
    ptr += F("<p style='text-align:center;color:#6c757d;margin-bottom:15px'>Current: <b>");
    ptr += currentHostname;
    ptr += F("</b></p>"); 
  }
  ptr += F("<form action='/update-hostname' method='get'><input type='hidden' name='token' value='");
  ptr += activeSessionToken;
  ptr += F("'><input type='text' name='newhostname' placeholder='New Hostname' required maxlength='20'>");
  ptr += F("<input type='submit' value='UPDATE'></form>");
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_ConnectWifi(String message = "") {
  extern String activeSessionToken;
  String token = "<input type='hidden' name='token' value='" + activeSessionToken + "'>";
  
  String ptr;
  ptr.reserve(1500);
  ptr = Helper_HttpHeader();
  ptr += F("<h1>WiFi Setup</h1><h5>Network connection</h5>");
  if(message.length() > 0) { ptr += F("<h6>"); ptr += message; ptr += F("</h6>"); }
  
  // Show current WiFi status
  ptr += F("<div style='background:#f8f9fa;padding:15px;border-radius:8px;margin-bottom:20px'><p style='margin:0;color:#495057'><b>Status:</b> ");
  if(WiFi.status() == WL_CONNECTED) {
    ptr += F("<span style='color:#28a745'>Connected</span><br><b>SSID:</b> ");
    ptr += WiFi.SSID();
    ptr += F("<br><b>IP:</b> ");
    ptr += WiFi.localIP().toString();
  } else {
    ptr += F("<span style='color:#dc3545'>Disconnected</span>");
  }
  ptr += F("</p><p style='margin:8px 0 0 0;color:#495057'><b>Auto-Connect:</b> ");
  ptr += Config_LoadWifiAuto() ? F("<span style='color:#28a745'>Enabled</span>") : F("<span style='color:#6c757d'>Disabled</span>");
  ptr += F("</p></div>");
  
  // Connection controls
  if(WiFi.status() == WL_CONNECTED) {
    ptr += F("<form action='/wifi-disconnect' method='get'>");
    ptr += token;
    ptr += F("<input type='submit' value='Disconnect' style='background:#dc3545'></form>");
  } else {
    String savedSSID = Config_LoadWifiSSID();
    if(savedSSID.length() > 0) {
      ptr += F("<form action='/wifi-manual-connect' method='get'>");
      ptr += token;
      ptr += F("<input type='submit' value='Connect to: ");
      ptr += savedSSID;
      ptr += F("' style='background:#28a745'></form>");
    }
  }
  
  // Auto-connect toggle
  ptr += F("<form action='/wifi-toggle-auto' method='get'>");
  ptr += token;
  if(Config_LoadWifiAuto()) {
    ptr += F("<input type='submit' value='Disable Auto-Connect' style='background:#6c757d'></form>");
  } else {
    ptr += F("<input type='submit' value='Enable Auto-Connect' style='background:#17a085'></form>");
  }
  
  // WiFi credentials form
  ptr += F("<h5 style='margin-top:30px;margin-bottom:15px'>Update Credentials</h5><form action='/save-wifi' method='get'>");
  ptr += token;
  ptr += F("<input type='text' name='wifiname' placeholder='WiFi Name' required>");
  ptr += F("<input type='password' name='wifipass' placeholder='WiFi Password (leave blank for open networks)'>");
  ptr += F("<input type='submit' value='SAVE & CONNECT'></form>");
  
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_ManageUsers(String message = "", int page = 0) {
  extern String activeSessionToken;
  
  const int USERS_PER_PAGE = 10;
  int skipCount = page * USERS_PER_PAGE;
  
  // Use char arrays for UIDs to save memory (8 chars max + null)
  char userFiles[USERS_PER_PAGE][17];
  char userNicks[USERS_PER_PAGE][13];  // 12 chars + null
  int displayCount = 0;
  int totalCount = 0;
  int skipped = 0;
  
  // Initialize arrays
  for(int i = 0; i < USERS_PER_PAGE; i++) {
    userFiles[i][0] = '\0';
    userNicks[i][0] = '\0';
  }
  
  Config_SelectSDCard();
  File usersDir = SD.open("/users");
  
  if(usersDir && usersDir.isDirectory()) {
    File entry = usersDir.openNextFile();
    
    while(entry) {
      if(!entry.isDirectory()) {
        // Get filename directly without creating String objects
        const char* fullName = entry.name();
        const char* fileName = fullName;
        
        // Find last slash
        for(const char* p = fullName; *p; p++) {
          if(*p == '/') fileName = p + 1;
        }
        
        int len = strlen(fileName);
        if(strcmp(fileName, "admin") != 0 && len > 0 && len <= 16) {
          totalCount++;
          
          if(skipped < skipCount) {
            skipped++;
          } else if(displayCount < USERS_PER_PAGE) {
            // Copy filename to array
            strncpy(userFiles[displayCount], fileName, 16);
            userFiles[displayCount][16] = '\0';
            
            // Read nickname (first line of file)
            userNicks[displayCount][0] = '\0';
            if(entry.available()) {
              int i = 0;
              while(entry.available() && i < 12) {
                char c = entry.read();
                if(c == '\n' || c == '\r') break;
                userNicks[displayCount][i++] = c;
              }
              userNicks[displayCount][i] = '\0';
            }
            displayCount++;
          }
        }
      }
      entry.close();
      entry = usersDir.openNextFile();
      
      // Yield every 10 files to prevent watchdog
      if(totalCount % 10 == 0) yield();
    }
    usersDir.close();
  }
  Config_DeselectAll();
  
  // Calculate pagination info
  int totalPages = (totalCount + USERS_PER_PAGE - 1) / USERS_PER_PAGE;
  if(totalPages < 1) totalPages = 1;
  int startNum = skipCount + 1;
  int endNum = skipCount + displayCount;
  
  // Pre-build token string once
  String token = "<input type='hidden' name='token' value='" + activeSessionToken + "'>";
  String pageHidden = "<input type='hidden' name='page' value='" + String(page) + "'>";
  
  // Build HTML - reserve memory upfront
  String ptr;
  ptr.reserve(3000);  // Pre-allocate to reduce fragmentation
  
  ptr = Helper_HttpHeader();
  ptr += F("<h1>Manage Users</h1><h5>Total: ");
  ptr += String(totalCount);
  ptr += F(" cards</h5>");
  if(message.length() > 0) { ptr += F("<h6>"); ptr += message; ptr += F("</h6>"); }
  
  // Add User button
  ptr += F("<form action='/add-user' method='get' style='margin-bottom:15px'>");
  ptr += token;
  ptr += F("<input type='submit' value='+ Add New User' style='background:#27ae60'></form>");
  
  // Pagination header
  if(totalCount > USERS_PER_PAGE) {
    ptr += F("<div style='text-align:center;margin-bottom:15px;font-size:13px;color:#495057'>Page ");
    ptr += String(page + 1) + "/" + String(totalPages);
    ptr += F("</div>");
  }
  
  ptr += F("<div style='text-align:left;font-size:13px;line-height:1.8'>");
  
  if(displayCount > 0) {
    // Table header
    ptr += F("<div style='display:flex;padding:8px 10px;border-bottom:2px solid #17a085;font-weight:bold;font-size:11px;color:#17a085'>");
    ptr += F("<span style='flex:2'>UID</span><span style='flex:2'>Nickname</span><span style='flex:1;text-align:right'>Actions</span></div>");
    
    for(int i = 0; i < displayCount; i++) {
      const char* nick = (userNicks[i][0] != '\0') ? userNicks[i] : "-";
      ptr += F("<div style='display:flex;align-items:center;padding:8px 10px;border-bottom:1px solid #e9ecef'>");
      ptr += F("<span style='flex:2;font-weight:500;color:#495057;font-size:11px'>");
      ptr += String(startNum + i) + ". " + userFiles[i];
      ptr += F("</span><span style='flex:2;color:#6c757d;font-size:11px'>");
      ptr += nick;
      ptr += F("</span><div style='flex:1;display:flex;gap:5px;justify-content:flex-end'>");
      
      // Edit button
      ptr += F("<form action='/edit-user' method='get' style='margin:0'>");
      ptr += token;
      ptr += F("<input type='hidden' name='uid' value='");
      ptr += userFiles[i];
      ptr += F("'>");
      ptr += pageHidden;
      ptr += F("<input type='submit' value='Edit' style='padding:5px 8px;font-size:10px;background:#17a085;width:auto;margin:0'></form>");
      
      // Delete button
      ptr += F("<form action='/delete-user' method='get' style='margin:0'>");
      ptr += token;
      ptr += F("<input type='hidden' name='uid' value='");
      ptr += userFiles[i];
      ptr += F("'>");
      ptr += pageHidden;
      ptr += F("<input type='submit' value='Del' style='padding:5px 8px;font-size:10px;background:#dc3545;width:auto;margin:0'></form></div></div>");
      
      yield();  // Allow background tasks after each row
    }
  } else {
    ptr += F("<p style='text-align:center;color:#6c757d;padding:20px'>No users registered</p>");
  }
  
  ptr += F("</div>");
  
  // Pagination buttons
  if(totalPages > 1) {
    ptr += F("<div style='display:flex;gap:10px;margin-top:15px'>");
    
    // Previous button
    if(page > 0) {
      ptr += F("<form action='/manage-users' method='get' style='flex:1;margin:0'>");
      ptr += token;
      ptr += F("<input type='hidden' name='page' value='");
      ptr += String(page - 1);
      ptr += F("'><input type='submit' value='Previous' style='background:#6c757d;width:100%'></form>");
    } else {
      ptr += F("<div style='flex:1'></div>");
    }
    
    // Next button
    if(page < totalPages - 1) {
      ptr += F("<form action='/manage-users' method='get' style='flex:1;margin:0'>");
      ptr += token;
      ptr += F("<input type='hidden' name='page' value='");
      ptr += String(page + 1);
      ptr += F("'><input type='submit' value='Next' style='background:#17a085;width:100%'></form>");
    } else {
      ptr += F("<div style='flex:1'></div>");
    }
    
    ptr += F("</div>");
  }
  
  // Clear All Users section
  if(totalCount > 0) {
    ptr += F("<div style='margin:20px 0 10px;padding:15px;background:#fff3cd;border-radius:8px;border:1px solid #ffc107'>");
    ptr += F("<p style='margin:0 0 10px 0;color:#856404;font-weight:bold;font-size:14px'>Danger Zone</p>");
    ptr += F("<form action='/clear-all-users' method='get'>");
    ptr += token;
    ptr += F("<input type='password' name='adminpass' placeholder='Admin password' style='margin-bottom:10px' required>");
    ptr += F("<input type='submit' value='Clear All (");
    ptr += String(totalCount);
    ptr += F(" users)' style='background:#dc3545'></form></div>");
  }
  
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_EditUser(String uid, String nickname = "", String message = "", int page = 0) {
  extern String activeSessionToken;
  
  String ptr = Helper_HttpHeader();
  ptr += F("<h1>Edit User</h1><h5>UID: ");
  ptr += uid;
  ptr += F("</h5>");
  if(message.length() > 0) { ptr += F("<h6>"); ptr += message; ptr += F("</h6>"); }
  
  ptr += F("<form action='/save-user' method='get'><input type='hidden' name='token' value='");
  ptr += activeSessionToken;
  ptr += F("'><input type='hidden' name='uid' value='");
  ptr += uid;
  ptr += F("'><input type='hidden' name='page' value='");
  ptr += String(page);
  ptr += F("'><input type='text' name='nickname' value='");
  ptr += nickname;
  ptr += F("' placeholder='Enter nickname (max 20 chars)' maxlength='20'><input type='submit' value='Save Nickname'></form>");
  
  ptr += F("<form action='/manage-users' method='get' style='margin-top:10px'><input type='hidden' name='token' value='");
  ptr += activeSessionToken;
  ptr += F("'><input type='hidden' name='page' value='");
  ptr += String(page);
  ptr += F("'><input type='submit' value='Back to Users' style='background:#6c757d'></form>");
  
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_AddUser(String message = "") {
  extern String activeSessionToken;
  
  String ptr = Helper_HttpHeader();
  ptr += F("<h1>Add User</h1><h5>Register New Card Manually</h5>");
  if(message.length() > 0) { ptr += F("<h6>"); ptr += message; ptr += F("</h6>"); }
  
  ptr += F("<form action='/create-user' method='get'><input type='hidden' name='token' value='");
  ptr += activeSessionToken;
  ptr += F("'><input type='text' name='uid' placeholder='RFID Key (8 hex chars)' maxlength='16' required pattern='[A-Fa-f0-9]+' style='text-transform:uppercase'>");
  ptr += F("<input type='text' name='nickname' placeholder='Nickname (optional)' maxlength='20'>");
  ptr += F("<input type='submit' value='Add User' style='background:#27ae60'></form>");
  
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_BootLogs() {
  String ptr;
  ptr.reserve(1500);
  ptr = Helper_HttpHeader();
  ptr += F("<h1>Boot Logs</h1><h5>Initialization Messages</h5>");
  
  if(bootLogCount > 0) {
    ptr += F("<div style='text-align:left;font-size:13px;line-height:1.8;color:#495057'>");
    
    for(int i = 0; i < bootLogCount; i++) {
      ptr += F("<div style='padding:8px;border-bottom:1px solid #e9ecef;font-family:monospace;font-size:12px'>");
      ptr += String(i + 1);
      ptr += F(". ");
      ptr += bootLogs[i];
      ptr += F("</div>");
    }
    
    ptr += F("</div>");
  } else {
    ptr += F("<p style='text-align:center;color:#6c757d;padding:20px'>No boot logs available</p>");
  }
  
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_SystemInfo() {
  String deviceName = Config_LoadHostname();
  if(deviceName.length() == 0) deviceName = F("Not Set");
  
  String ptr;
  ptr.reserve(2000);
  ptr = Helper_HttpHeader();
  ptr += F("<h1>System Info</h1><h5>ESP8266 Statistics</h5>");
  ptr += F("<div style='text-align:left;font-size:13px;line-height:1.8;color:#495057;margin-bottom:15px'>");  
  
  // Device Name
  ptr += F("<b>Device Name:</b><br>");
  ptr += deviceName;
  ptr += F("<br><br><b>Firmware:</b><br>Version: ");
  ptr += APP_VERSION;
  ptr += F("<br>Build: ");
  ptr += __DATE__;
  ptr += F(" ");
  ptr += __TIME__;
  
  // Hardware Status
  ptr += F("<br><br><b>Hardware Status:</b><br>");
  ptr += F("RFID Reader: <span style='color:");
  ptr += rfidStatus ? F("#28a745'>✓ OK") : F("#dc3545'>✗ Error");
  ptr += F("</span><br>SD Card: <span style='color:");
  ptr += sdCardStatus ? F("#28a745'>✓ OK") : F("#dc3545'>✗ Error");
  ptr += F("</span><br>LCD Display: <span style='color:");
  ptr += displayStatus ? F("#28a745'>✓ OK") : F("#dc3545'>✗ Error");
  ptr += F("</span><br>WiFi Module: <span style='color:");
  ptr += wifiStatus ? F("#28a745'>✓ OK") : F("#dc3545'>✗ Error");
  ptr += F("</span>");
  
  // Flash Memory Info
  uint32_t flashSize = ESP.getFlashChipSize();
  uint32_t sketchSize = ESP.getSketchSize();
  uint32_t freeSpace = ESP.getFreeSketchSpace();
  
  ptr += F("<br><br><b>Flash Memory:</b><br>Total: ");
  ptr += String(flashSize / 1024);
  ptr += F(" KB<br>Used: ");
  ptr += String(sketchSize / 1024);
  ptr += F(" KB<br>Free: ");
  ptr += String(freeSpace / 1024);
  ptr += F(" KB<br>Usage: ");
  ptr += String((sketchSize * 100.0) / flashSize, 1);
  ptr += F("%");
  
  // Heap Memory
  ptr += F("<br><br><b>Heap Memory:</b><br>Free: ");
  ptr += String(ESP.getFreeHeap() / 1024);
  ptr += F(" KB");
  
  // System Info
  ptr += F("<br><br><b>System:</b><br>Chip ID: ");
  ptr += String(ESP.getChipId(), HEX);
  ptr += F("<br>CPU Freq: ");
  ptr += String(ESP.getCpuFreqMHz());
  ptr += F(" MHz<br>SDK: ");
  ptr += ESP.getSdkVersion();
  
  // WiFi Info
  ptr += F("<br><br><b>WiFi:</b><br>Mode: ");
  ptr += (WiFi.getMode() == WIFI_AP) ? F("AP") : F("STA");
  if(WiFi.getMode() == WIFI_AP) {
    String hostname = Config_LoadHostname();
    ptr += F("<br>SSID: ");
    ptr += APP_NAME;
    if(hostname.length() > 0) { ptr += F(" - "); ptr += hostname; }
    ptr += F("<br>IP: 192.168.4.1");
  }
  
  ptr += F("</div>");
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}