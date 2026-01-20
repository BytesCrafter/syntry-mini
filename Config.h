
//Your Domain name with URL path or IP address with path
String serverName = "https://system.bytescrafter.net/v1/api/users/signin";

#define APP_VERSION "1.4.0" 
#define BUILD_DATE "2026-01-20"

#define BAUD_RATE 9600 

#define BUZZER 15 // D8 - pin where buzzer is connect!
#define CONFIG_RELAY 1 // TX - pin where RELAY is connect

#define LIGHT_RED 9 
#define LIGHT_GREEN 8 

#define WAIT_OPEN 2000 

#define RFID_CS_PIN 0 // D3 - Configurable, see typical pin layout above /def 4
#define SDCARD_CS_PIN 2 // D4 - 

bool isLoaded = false;

// Hardware initialization status
bool sdCardStatus = false;
bool rfidStatus = false;
bool displayStatus = false;
bool wifiStatus = false;

String rfidMode = "access"; //default mode.
String Rfid_Status() {
  return rfidMode;
}

// EEPROM Password Management
#define EEPROM_SIZE 512
#define EEPROM_PASS_ADDR 0
#define EEPROM_PASS_MAX_LEN 32

void Config_Init() {
  EEPROM.begin(EEPROM_SIZE);
  
  // // Make sure CS lines are outputs and start HIGH (inactive)
  // pinMode(SDCARD_CS_PIN, OUTPUT); digitalWrite(SDCARD_CS_PIN, HIGH);

  // // Make sure CS lines are outputs and start HIGH (inactive)
  // pinMode(RFID_CS_PIN, OUTPUT); digitalWrite(RFID_CS_PIN, HIGH);
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