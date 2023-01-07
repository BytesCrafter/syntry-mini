//Your Domain name with URL path or IP address with path
String serverName = "https://system.bytescrafter.net/v1/api/users/signin";

#define APP_VERSION "1.0.0" 
#define RST_KEYTAG "1502774F" 

#define BAUD_RATE 115200 

#define BUZZER 15 //pin where buzzer is connect!
#define CONFIG_RELAY 1 //pin where RELAY is connect

#define LIGHT_RED 9 
#define LIGHT_GREEN 8 

#define WAIT_OPEN 7000 

#define SDCARD_PIN 0

unsigned long initTime;
unsigned long timeSpan = 2000;
bool isInit = false;

void Config_Init() {
  initTime = millis();

  SPI.begin();
  delay(100);
  
  Serial.begin(BAUD_RATE);
  delay(250);
}