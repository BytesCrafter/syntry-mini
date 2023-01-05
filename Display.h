//DISPLAY LIBRARIES
#include <LiquidCrystal_I2C.h>

// DISPLAY: Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2); //16, 2 OR 20, 4

void Display_Detect() {
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
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
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(5000);          
}

void Display_Init() {
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
}

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

void Display_Sample() {
  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  // print message
  lcd.print("FIRST ROW: 123");
  delay(500);

  // clears the display to print new message
  lcd.clear();

  // set cursor to first column, second row
  lcd.setCursor(0,1);
  lcd.print("SECOND ROW: @$&");
  delay(500);
  lcd.clear(); 
}