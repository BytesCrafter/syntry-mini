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

String Helper_HttpHeader() {
  String ptr = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">";
  ptr +="<title>Syntry</title><style>";
  ptr +="*{margin:0;padding:0;box-sizing:border-box}";
  ptr +="body{font-family:Arial,sans-serif;background:linear-gradient(135deg,#1ab394 0%,#26d0b0 100%);min-height:100vh;display:flex;align-items:center;justify-content:center;padding:20px}";
  ptr +=".c{background:#fff;border-radius:16px;box-shadow:0 10px 40px rgba(0,0,0,.15);padding:40px;max-width:500px;width:100%}";
  ptr +="h1{color:#1a1a1a;font-size:28px;margin-bottom:8px;text-align:center;font-weight:700}";
  ptr +="h5{color:#6c757d;font-size:14px;margin-bottom:30px;text-align:center;font-weight:400}";
  ptr +="h6{color:#dc3545;font-size:13px;margin-bottom:15px;text-align:center}";
  ptr +="input[type=text],input[type=password]{width:100%;padding:14px;border:2px solid #e9ecef;border-radius:12px;font-size:15px;margin-bottom:16px;transition:.3s;background:#f8f9fa}";
  ptr +="input:focus{outline:none;border-color:#1ab394;background:#fff}";
  ptr +="input[type=submit]{width:100%;padding:16px;background:#1ab394;color:#fff;border:none;border-radius:12px;font-size:16px;font-weight:600;cursor:pointer;transition:.3s;margin-bottom:12px;box-shadow:0 4px 15px rgba(26,179,148,.3)}";
  ptr +="input[type=submit]:hover{background:#17a085;transform:translateY(-2px);box-shadow:0 6px 20px rgba(26,179,148,.4)}";
  ptr +="input[type=submit]:active{transform:translateY(0)}";
  ptr +=".grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-bottom:20px}";
  ptr +=".grid form{margin:0}";
  ptr +=".grid input[type=submit]{height:67px;margin:0;font-size:15px;padding:10px}";
  ptr +=".split{display:grid;grid-template-columns:1fr 1fr;gap:12px}";
  ptr +=".split form{margin:0}";
  ptr +=".split input[type=submit]{margin:0}";
  ptr +="</style></head><body><div class=\"c\">";
  return ptr;
}

String Helper_HttpFooter() {
  String ptr = "</div></body></html>";
  return ptr;
}

String Helper_HttpBackToMenu() {
  extern String activeSessionToken;
  String ptr = "<form action='/menu' method='get'>";
  ptr += "<input type='hidden' name='token' value='" + activeSessionToken + "'>";
  ptr += "<input type='submit' value='Back to Menu'></form>";
  return ptr;
}

String Helper_Hotspot_Login(String message = "") {
  String ptr = Helper_HttpHeader();
  ptr += String("<h1>") + APP_NAME + "</h1><h5>Powered by ERPat System</h5>";
  if(message != "") ptr +="<h6>"+message+"</h6>";
  ptr +="<form action='/login' method='get'>";
  ptr +="<input type='text' name='uname' placeholder='Username' required>";
  ptr +="<input type='password' name='pword' placeholder='Password' required>";
  ptr +="<input type='submit' value='LOGIN'>";
  ptr +=Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_To_Menu() {
  extern String activeSessionToken;
  String token = "<input type='hidden' name='token' value='" + activeSessionToken + "'>";
  
  String ptr = Helper_HttpHeader();
  ptr += String("<h1>") + APP_NAME + "</h1><h5>Control Panel</h5>";
  ptr +="<div class='grid'>";
  ptr +="<form action='/verify' method='get'>" + token + "<input type='submit' value='Verify Mode'></form>";
  ptr +="<form action='/access' method='get'>" + token + "<input type='submit' value='Access Mode'></form>";
  ptr +="<form action='/add' method='get'>" + token + "<input type='submit' value='Add Card'></form>";
  ptr +="<form action='/remove' method='get'>" + token + "<input type='submit' value='Remove Card'></form>";
  ptr +="<form action='/change-password' method='get'>" + token + "<input type='submit' value='Change Password'></form>";
  ptr +="<form action='/change-hostname' method='get'>" + token + "<input type='submit' value='Change Hostname'></form>";
  ptr +="<form action='/boot-logs' method='get'>" + token + "<input type='submit' value='Boot Logs' style='background:#17a2b8'></form>";
  ptr +="<form action='/wifi-connect' method='get'>" + token + "<input type='submit' value='WiFi Setup' style='background:#3498db'></form>";
  int userCount = SDCard_CountUsers();
  ptr +="<form action='/manage-users' method='get'>" + token + "<input type='submit' value='Manage Users (" + String(userCount) + ")' style='background:#8e44ad'></form>";
  ptr +="<form action='/system' method='get'>" + token + "<input type='submit' value='System Info' style='background:#f39c12'></form>";
  ptr +="</div>";
  ptr +="<div class='split'>";
  ptr +="<form action='/menu' method='get'>" + token + "<input type='hidden' name='action' value='restart'><input type='submit' value='Restart'></form>";
  ptr +="<form action='/logout' method='get'>" + token + "<input type='submit' value='Logout' style='background:#e53e3e'></form>";
  ptr +="</div>";
  ptr +=Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_To_Access() {
  extern String activeSessionToken;
  String token = "<input type='hidden' name='token' value='" + activeSessionToken + "'>";
  
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Access Mode</h1><h5>Tap your RFID card</h5>";
  ptr +="<form action='/access' method='get'>" + token + "<input type='hidden' name='action' value='override'><input type='submit' value='Override Access' style='background:#48bb78'></form>";
  ptr +=Helper_HttpBackToMenu();
  ptr +=Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_To_Add() {
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Add Card Mode</h1><h5>Tap card to register</h5>";
  ptr +=Helper_HttpBackToMenu();
  ptr +=Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_To_Remove() {
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Remove Card Mode</h1><h5>Tap card to delete</h5>";
  ptr +=Helper_HttpBackToMenu();
  ptr +=Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_To_Verify() {
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Verify Mode</h1><h5>Tap card to check</h5>";
  ptr +=Helper_HttpBackToMenu();
  ptr +=Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_ChangePassword(String message = "") {
  extern String activeSessionToken;
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Change Password</h1><h5>Update admin credentials</h5>";
  if(message != "") ptr +="<h6>"+message+"</h6>";
  ptr +="<form action='/update-password' method='get'>";
  ptr += "<input type='hidden' name='token' value='" + activeSessionToken + "'>";
  ptr +="<input type='password' name='newpass' placeholder='New Password' required>";
  ptr +="<input type='password' name='confirmpass' placeholder='Confirm Password' required>";
  ptr +="<input type='submit' value='UPDATE'></form>";
  ptr +=Helper_HttpBackToMenu();
  ptr +=Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_ChangeHostname(String message = "") {
  extern String activeSessionToken;
  // Get current hostname from EEPROM
  String currentHostname = Config_LoadHostname();
  
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Change Hostname</h1><h5>Update device name</h5>";
  if(message != "") ptr +="<h6>"+message+"</h6>";
  if(currentHostname != "") ptr +="<p style='text-align:center;color:#6c757d;margin-bottom:15px'>Current: <b>"+currentHostname+"</b></p>";
  ptr +="<form action='/update-hostname' method='get'>";
  ptr += "<input type='hidden' name='token' value='" + activeSessionToken + "'>";
  ptr +="<input type='text' name='newhostname' placeholder='New Hostname' required maxlength='20'>";
  ptr +="<input type='submit' value='UPDATE'></form>";
  ptr +=Helper_HttpBackToMenu();
  ptr +=Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_ConnectWifi(String message = "") {
  extern String activeSessionToken;
  String token = "<input type='hidden' name='token' value='" + activeSessionToken + "'>";
  
  String ptr = Helper_HttpHeader();
  ptr +="<h1>WiFi Setup</h1><h5>Network connection</h5>";
  if(message != "") ptr +="<h6>"+message+"</h6>";
  
  // Show current WiFi status
  ptr +="<div style='background:#f8f9fa;padding:15px;border-radius:8px;margin-bottom:20px'>";
  ptr +="<p style='margin:0;color:#495057'><b>Status:</b> ";
  if(WiFi.status() == WL_CONNECTED) {
    ptr +="<span style='color:#28a745'>Connected</span><br>";
    ptr +="<b>SSID:</b> " + WiFi.SSID() + "<br>";
    ptr +="<b>IP:</b> " + WiFi.localIP().toString() + "";
  } else {
    ptr +="<span style='color:#dc3545'>Disconnected</span>";
  }
  ptr +="</p><p style='margin:8px 0 0 0;color:#495057'><b>Auto-Connect:</b> ";
  ptr += Config_LoadWifiAuto() ? "<span style='color:#28a745'>Enabled</span>" : "<span style='color:#6c757d'>Disabled</span>";
  ptr +="</p></div>";
  
  // Connection controls
  if(WiFi.status() == WL_CONNECTED) {
    ptr +="<form action='/wifi-disconnect' method='get'>" + token + "<input type='submit' value='Disconnect' style='background:#dc3545'></form>";
  } else {
    String savedSSID = Config_LoadWifiSSID();
    if(savedSSID != "") {
      ptr +="<form action='/wifi-manual-connect' method='get'>" + token + "<input type='submit' value='Connect to: " + savedSSID + "' style='background:#28a745'></form>";
    }
  }
  
  // Auto-connect toggle
  if(Config_LoadWifiAuto()) {
    ptr +="<form action='/wifi-toggle-auto' method='get'>" + token + "<input type='submit' value='Disable Auto-Connect' style='background:#6c757d'></form>";
  } else {
    ptr +="<form action='/wifi-toggle-auto' method='get'>" + token + "<input type='submit' value='Enable Auto-Connect' style='background:#17a085'></form>";
  }
  
  // WiFi credentials form
  ptr +="<h5 style='margin-top:30px;margin-bottom:15px'>Update Credentials</h5>";
  ptr +="<form action='/save-wifi' method='get'>";
  ptr += token;
  ptr +="<input type='text' name='wifiname' placeholder='WiFi Name' required>";
  ptr +="<input type='password' name='wifipass' placeholder='WiFi Password' required>";
  ptr +="<input type='submit' value='SAVE & CONNECT'></form>";
  
  ptr +=Helper_HttpBackToMenu();
  ptr +=Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_ManageUsers(String message = "") {
  extern String activeSessionToken;
  String token = "<input type='hidden' name='token' value='" + activeSessionToken + "'>";
  
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Manage Users</h1><h5>Registered RFID Cards</h5>";
  if(message != "") ptr +="<h6>"+message+"</h6>";
  
  // Read all user files from SD card
  Config_SelectSDCard();
  File usersDir = SD.open("/users");
  
  if(usersDir && usersDir.isDirectory()) {
    ptr +="<div style='text-align:left;font-size:13px;line-height:1.8'>";
    
    int userCount = 0;
    File entry = usersDir.openNextFile();
    
    while(entry) {
      if(!entry.isDirectory()) {
        String fileName = String(entry.name());
        // Remove path prefix if present
        int lastSlash = fileName.lastIndexOf('/');
        if(lastSlash >= 0) {
          fileName = fileName.substring(lastSlash + 1);
        }
        
        if(fileName != "admin" && fileName.length() > 0) {
          ptr +="<div style='display:flex;justify-content:space-between;align-items:center;padding:10px;border-bottom:1px solid #e9ecef'>";
          ptr +="<span style='font-weight:500;color:#495057'>"+fileName+"</span>";
          ptr +="<form action='/delete-user' method='get' style='margin:0'>";
          ptr += token;
          ptr +="<input type='hidden' name='uid' value='"+fileName+"'>";
          ptr +="<input type='submit' value='Delete' style='padding:6px 12px;font-size:12px;background:#dc3545;width:auto;margin:0'>";
          ptr +="</form></div>";
          userCount++;
        }
      }
      entry.close();
      entry = usersDir.openNextFile();
    }
    
    if(userCount == 0) {
      ptr +="<p style='text-align:center;color:#6c757d;padding:20px'>No users registered</p>";
    }
    
    ptr +="</div>";
    usersDir.close();
  } else {
    ptr +="<p style='text-align:center;color:#dc3545'>Error reading users directory</p>";
  }
  
  Config_DeselectAll();
  
  ptr +=Helper_HttpBackToMenu();
  ptr +=Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_BootLogs() {
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Boot Logs</h1><h5>Initialization Messages</h5>";
  
  if(bootLogCount > 0) {
    ptr +="<div style='text-align:left;font-size:13px;line-height:1.8;color:#495057'>";
    
    for(int i = 0; i < bootLogCount; i++) {
      ptr +="<div style='padding:8px;border-bottom:1px solid #e9ecef;font-family:monospace;font-size:12px'>";
      ptr += String(i + 1) + ". " + bootLogs[i];
      ptr +="</div>";
    }
    
    ptr +="</div>";
  } else {
    ptr +="<p style='text-align:center;color:#6c757d;padding:20px'>No boot logs available</p>";
  }
  
  ptr +=Helper_HttpBackToMenu();
  ptr +=Helper_HttpFooter();
  return ptr;
}

String Helper_Hotspot_SystemInfo() {
  // Get device name from EEPROM
  String deviceName = Config_LoadHostname();
  if(deviceName == "") deviceName = "Not Set";
  
  String ptr = Helper_HttpHeader();
  ptr +="<h1>System Info</h1><h5>ESP8266 Statistics</h5>";
  ptr +="<div style='text-align:left;font-size:13px;line-height:1.8;color:#495057'>";  
  
  // Device Name
  ptr +="<b>Device Name:</b><br>";
  ptr += deviceName + "<br><br>";
  
  // Firmware Version
  ptr +="<b>Firmware:</b><br>";
  ptr +="Version: " + String(APP_VERSION) + "<br>";
  ptr +="Build: " + String(__DATE__) + " " + String(__TIME__) + "<br><br>";
  
  // Hardware Status
  ptr +="<b>Hardware Status:</b><br>";
  ptr +="RFID Reader: <span style='color:" + String(rfidStatus ? "#28a745" : "#dc3545") + "'>"
       + String(rfidStatus ? "✓ OK" : "✗ Error") + "</span><br>";
  ptr +="SD Card: <span style='color:" + String(sdCardStatus ? "#28a745" : "#dc3545") + "'>"
       + String(sdCardStatus ? "✓ OK" : "✗ Error") + "</span><br>";
  ptr +="LCD Display: <span style='color:" + String(displayStatus ? "#28a745" : "#dc3545") + "'>"
       + String(displayStatus ? "✓ OK" : "✗ Error") + "</span><br>";
  ptr +="WiFi Module: <span style='color:" + String(wifiStatus ? "#28a745" : "#dc3545") + "'>"
       + String(wifiStatus ? "✓ OK" : "✗ Error") + "</span><br><br>";
  
  // Flash Memory Info
  uint32_t flashSize = ESP.getFlashChipSize();
  uint32_t sketchSize = ESP.getSketchSize();
  uint32_t freeSpace = ESP.getFreeSketchSpace();
  float flashUsage = (sketchSize * 100.0) / flashSize;
  
  ptr +="<b>Flash Memory:</b><br>";
  ptr +="Total: " + String(flashSize / 1024) + " KB<br>";
  ptr +="Used: " + String(sketchSize / 1024) + " KB<br>";
  ptr +="Free: " + String(freeSpace / 1024) + " KB<br>";
  ptr +="Usage: " + String(flashUsage, 1) + "%<br><br>";
  
  // Heap Memory
  ptr +="<b>Heap Memory:</b><br>";
  ptr +="Free: " + String(ESP.getFreeHeap() / 1024) + " KB<br><br>";
  
  // System Info
  ptr +="<b>System:</b><br>";
  ptr +="Chip ID: " + String(ESP.getChipId(), HEX) + "<br>";
  ptr +="CPU Freq: " + String(ESP.getCpuFreqMHz()) + " MHz<br>";
  ptr +="SDK: " + String(ESP.getSdkVersion()) + "<br><br>";
  
  // WiFi Info
  ptr +="<b>WiFi:</b><br>";
  ptr +="Mode: " + String(WiFi.getMode() == WIFI_AP ? "AP" : "STA") + "<br>";
  if(WiFi.getMode() == WIFI_AP) {
    // Read hostname from EEPROM
    String hostname = Config_LoadHostname();
    ptr += String("SSID: ") + APP_NAME;
    if(hostname != "") ptr +=" - " + hostname;
    ptr +="<br>IP: 192.168.4.1<br>";
  }
  
  ptr +="</div>";
  ptr +=Helper_HttpBackToMenu();
  ptr +=Helper_HttpFooter();
  return ptr;
}