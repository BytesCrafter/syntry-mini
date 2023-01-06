//Your Domain name with URL path or IP address with path
String serverName = "https://system.bytescrafter.net/v1/api/users/signin";

#define BAUD_RATE 115200 

#define BUZZER 15 //pin where buzzer is connect!
#define CONFIG_RELAY 1 //pin where RELAY is connect

#define LIGHT_RED 9 
#define LIGHT_GREEN 8 

#define WAIT_OPEN 7000 

#define SDCARD_PIN 0

void Config_Init() {
  SPI.begin();
  Serial.begin(BAUD_RATE);
}