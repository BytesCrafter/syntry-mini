/*
  Module: LiquidCrystal_I2C Module.
  Version: 0.1.0
*/

//Include the libray for display.
#include <LiquidCrystal_I2C.h>

//Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2); //16, 2 OR 20, 4

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
    Serial.println("LCD Display: OK");
  } else {
    Serial.println("LCD Error at 0x27");
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