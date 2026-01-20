/*
  Module: LiquidCrystal_I2C Module.
  Version: 0.1.0
*/

//Include the libray for display.
#include <LiquidCrystal_I2C.h>

//Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2); //16, 2 OR 20, 4
byte lcdAddress = 0x27; // Default address, will be detected

void Display_Detect() {
  byte error, address;
  int nDevices;
  Serial.println("Scanning display...");
  nDevices = 0;
  
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    } else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }

  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("done\n");
  }

  delay(5000);          
}

//Required to initialize the display.
void Display_Init() {
  // Initialize I2C bus with correct pins for ESP8266
  Wire.begin(4, 5); // SDA=GPIO4(D2), SCL=GPIO5(D1)
  delay(100); // Allow I2C to stabilize
  
  // Test I2C communication before initializing
  Wire.beginTransmission(0x27);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    // LCD found at address 0x27
    lcd.init(); //initialize LCD
    lcd.backlight(); //turn on LCD backlight
    
    // Try to print test to verify LCD is working
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LCD Testing...");
    delay(100);
    
    displayStatus = true;
    Config_AddBootLog("Display: LCD initialized (I2C 0x27)");
  } else {
    displayStatus = false;
    String errorMsg = "Display: LCD Error " + String(error) + " at 0x27";
    Config_AddBootLog(errorMsg);
    Serial.println(errorMsg + " (2=NACK addr, 3=NACK data, 4=other)");
  }
}

//Clear all Display rows.
void Display_Show(String first, String second = "", String third = "", String fourt = "") {
  lcd.clear();
  
  if(first) {
    lcd.setCursor(0, 0);
    lcd.print(first);
  }

  if(second) {
    lcd.setCursor(0,1);
    lcd.print(second);
  }

  if(third) {
    lcd.setCursor(0,2);
    lcd.print(third);
  }

  if(fourt) {
    lcd.setCursor(0,3);
    lcd.print(fourt);
  }
}