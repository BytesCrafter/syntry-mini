
//Your Domain name with URL path or IP address with path
String serverName = "https://system.bytescrafter.net/v1/api/users/signin";

#define APP_VERSION "1.0.0" 
#define RST_KEYTAG "80BA9126" 

#define BAUD_RATE 9600 

#define BUZZER 15 //pin where buzzer is connect!
#define CONFIG_RELAY 1 //pin where RELAY is connect

#define LIGHT_RED 9 
#define LIGHT_GREEN 8 

#define WAIT_OPEN 2000 

#define SDCARD_PIN 2

bool isInit = false;

void Config_Init() {
  //Do something!
}

void Config_Loop() {
  if(Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if(command.equals("white")) {
      Serial.println("White");
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