
//Your Domain name with URL path or IP address with path
String serverName = "https://system.bytescrafter.net/v1/api/users/signin";

#define APP_NAME "Syntry Access"
#define APP_VERSION "1.4.0" 
#define BUILD_DATE "2026-01-20"

#define BAUD_RATE 9600 

#define BUZZER 0 // D3/GPIO0 - Buzzer pin (moved from D8)
// GPIO16 (D0) has no boot mode restrictions - perfect for relay!
#define CONFIG_RELAY 16 // D0/GPIO16 - RELAY moved here (was RFID RST)

#define LIGHT_RED 9 
#define LIGHT_GREEN 8 

#define WAIT_OPEN 2000 

#define RFID_CS_PIN 15 // D8 - Changed from D3, now used for RFID CS
#define SDCARD_CS_PIN 2 // D4 - 

bool isLoaded = false;

// Hardware initialization status
bool sdCardStatus = false;
bool rfidStatus = false;
bool displayStatus = false;
bool wifiStatus = false;

// Boot log storage (last 20 initialization messages)
#define MAX_BOOT_LOGS 20
String bootLogs[MAX_BOOT_LOGS];
int bootLogCount = 0;

void Config_AddBootLog(String message) {
  if(bootLogCount < MAX_BOOT_LOGS) {
    bootLogs[bootLogCount] = message;
    bootLogCount++;
  }
  Serial.println(message);
}

String rfidMode = "access"; //default mode.
String Rfid_Status() {
  return rfidMode;
}

// EEPROM Password Management
#define EEPROM_SIZE 512
#define EEPROM_PASS_ADDR 0
#define EEPROM_PASS_MAX_LEN 32

// EEPROM Hostname Management (stored after password)
#define EEPROM_HOST_ADDR 34
#define EEPROM_HOST_MAX_LEN 20

// EEPROM WiFi Credentials (stored after hostname)
#define EEPROM_WIFI_SSID_ADDR 56
#define EEPROM_WIFI_SSID_MAX_LEN 64
#define EEPROM_WIFI_PASS_ADDR 121
#define EEPROM_WIFI_PASS_MAX_LEN 64
#define EEPROM_WIFI_AUTO_ADDR 186  // 1 byte for auto-connect flag

void Config_Init() {
  EEPROM.begin(EEPROM_SIZE);
  
  // CRITICAL: Initialize CS pins HIGH (inactive) BEFORE SPI.begin()
  pinMode(SDCARD_CS_PIN, OUTPUT);
  digitalWrite(SDCARD_CS_PIN, HIGH);
  
  pinMode(RFID_CS_PIN, OUTPUT);
  digitalWrite(RFID_CS_PIN, HIGH);
  
  Serial.println("CS Pins initialized");
  
  // Initialize SPI bus for RFID and SD Card communication
  SPI.begin();
  Serial.println("SPI bus initialized");
}

// SPI Bus Management - Call before using RFID
void Config_SelectRFID() {
  digitalWrite(SDCARD_CS_PIN, HIGH);  // Deselect SD Card
  delayMicroseconds(10);
  digitalWrite(RFID_CS_PIN, LOW);     // Select RFID
}

// SPI Bus Management - Call before using SD Card
void Config_SelectSDCard() {
  digitalWrite(RFID_CS_PIN, HIGH);    // Deselect RFID
  delayMicroseconds(10);
  digitalWrite(SDCARD_CS_PIN, LOW);   // Select SD Card
}

// SPI Bus Management - Deselect all devices
void Config_DeselectAll() {
  digitalWrite(RFID_CS_PIN, HIGH);
  digitalWrite(SDCARD_CS_PIN, HIGH);
}

// Save password to EEPROM
bool Config_SavePassword(String password) {
  if(password.length() > EEPROM_PASS_MAX_LEN) {
    return false;
  }
  
  // Write password length
  EEPROM.write(EEPROM_PASS_ADDR, password.length());
  
  // Write password string
  for(int i = 0; i < password.length(); i++) {
    EEPROM.write(EEPROM_PASS_ADDR + 1 + i, password[i]);
  }
  
  EEPROM.commit();
  Serial.println("Password saved to EEPROM");
  return true;
}

// Load password from EEPROM
String Config_LoadPassword() {
  int len = EEPROM.read(EEPROM_PASS_ADDR);
  
  // If length is 0 or invalid, return default password
  if(len == 0 || len > EEPROM_PASS_MAX_LEN || len == 255) {
    Serial.println("No password in EEPROM, using default");
    return "admin";
  }
  
  String password = "";
  for(int i = 0; i < len; i++) {
    password += (char)EEPROM.read(EEPROM_PASS_ADDR + 1 + i);
  }
  
  Serial.println("Password loaded from EEPROM");
  return password;
}

// Save WiFi Auto-Connect setting to EEPROM
void Config_SaveWifiAuto(bool autoConnect) {
  EEPROM.write(EEPROM_WIFI_AUTO_ADDR, autoConnect ? 1 : 0);
  EEPROM.commit();
}

// Load WiFi Auto-Connect setting from EEPROM (default: false)
bool Config_LoadWifiAuto() {
  byte value = EEPROM.read(EEPROM_WIFI_AUTO_ADDR);
  return (value == 1);
}

// Save WiFi SSID to EEPROM
bool Config_SaveWifiSSID(String ssid) {
  if(ssid.length() > EEPROM_WIFI_SSID_MAX_LEN) {
    return false;
  }
  
  // Write SSID length
  EEPROM.write(EEPROM_WIFI_SSID_ADDR, ssid.length());
  
  // Write SSID string
  for(int i = 0; i < ssid.length(); i++) {
    EEPROM.write(EEPROM_WIFI_SSID_ADDR + 1 + i, ssid[i]);
  }
  
  EEPROM.commit();
  return true;
}

// Load WiFi SSID from EEPROM
String Config_LoadWifiSSID() {
  int length = EEPROM.read(EEPROM_WIFI_SSID_ADDR);
  
  // If length is invalid, return empty
  if(length == 0 || length > EEPROM_WIFI_SSID_MAX_LEN || length == 255) {
    return "";
  }
  
  String ssid = "";
  for(int i = 0; i < length; i++) {
    ssid += char(EEPROM.read(EEPROM_WIFI_SSID_ADDR + 1 + i));
  }
  
  return ssid;
}

// Save WiFi Password to EEPROM
bool Config_SaveWifiPassword(String password) {
  if(password.length() > EEPROM_WIFI_PASS_MAX_LEN) {
    return false;
  }
  
  // Write password length
  EEPROM.write(EEPROM_WIFI_PASS_ADDR, password.length());
  
  // Write password string
  for(int i = 0; i < password.length(); i++) {
    EEPROM.write(EEPROM_WIFI_PASS_ADDR + 1 + i, password[i]);
  }
  
  EEPROM.commit();
  return true;
}

// Load WiFi Password from EEPROM
String Config_LoadWifiPassword() {
  int length = EEPROM.read(EEPROM_WIFI_PASS_ADDR);
  
  // If length is invalid, return empty
  if(length == 0 || length > EEPROM_WIFI_PASS_MAX_LEN || length == 255) {
    return "";
  }
  
  String password = "";
  for(int i = 0; i < length; i++) {
    password += char(EEPROM.read(EEPROM_WIFI_PASS_ADDR + 1 + i));
  }
  
  return password;
}

// Save hostname to EEPROM
bool Config_SaveHostname(String hostname) {
  if(hostname.length() > EEPROM_HOST_MAX_LEN) {
    return false;
  }
  
  // Write hostname length
  EEPROM.write(EEPROM_HOST_ADDR, hostname.length());
  
  // Write hostname string
  for(int i = 0; i < hostname.length(); i++) {
    EEPROM.write(EEPROM_HOST_ADDR + 1 + i, hostname[i]);
  }
  
  EEPROM.commit();
  Serial.println("Hostname saved to EEPROM");
  return true;
}

// Get or generate hostname (accessible from Config.h)
// Forward declaration for Helper_RandomString (defined in Helper.h)
String Helper_RandomString();

// Load hostname from EEPROM
String Config_LoadHostname() {
  int len = EEPROM.read(EEPROM_HOST_ADDR);
  
  // If length is 0 or invalid, return empty string
  if(len == 0 || len > EEPROM_HOST_MAX_LEN || len == 255) {
    Serial.println("No hostname in EEPROM");
    return Helper_RandomString();
  }
  
  String hostname = "";
  for(int i = 0; i < len; i++) {
    hostname += (char)EEPROM.read(EEPROM_HOST_ADDR + 1 + i);
  }
  
  Serial.println("Hostname loaded from EEPROM: " + hostname);
  return hostname;
}

void Config_Loop() {
  if(Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if(command.equals("goto sethostname")) {
      rfidMode = "sethostname";
      Serial.println("TAP TO TAG ADMIN");
    } else if(command.equals("goto access")) {
      rfidMode = "access";
      Serial.println("TAP YOUR CARD MODE");
    } else if(command.equals("reset")) {
      ESP.reset();
    } else if(command.equals("restart")) {
      ESP.restart();
    }
  }
}

// //TODO: For testing!
// bool Config_SaveSetting(int addrOffset, const String &strToWrite) {
//   byte len = strToWrite.length();
//   EEPROM.write(addrOffset, len);
//   for (int i = 0; i < len; i++)
//   {
//     EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
//   }
//   return true;
// }

// //TODO: For testing!
// String Config_LoadSetting(int addrOffset) {
//   int newStrLen = EEPROM.read(addrOffset);
//   char data[newStrLen + 1];

//   for (int i = 0; i < newStrLen; i++)
//   {
//     data[i] = EEPROM.read(addrOffset + 1 + i);
//   }
//   data[newStrLen] = '\ 0'; // !!! NOTE !!! Remove the space between the slash "/" and "0" (I've added a space because otherwise there is a display bug)

//   return String(data);
// }