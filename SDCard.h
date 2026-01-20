/*
  Module: SDCard Module.
  Version: 0.2.0
  
  WARNING: If SD Card is powered by 5V, MISO line needs voltage level shifter
  or 220Ω resistor + 3.3V zener diode to protect ESP8266!
*/

//Initialize the SD Card.
void SDCard_Init(void (*callback)(String, String, String, String)) {
  Config_SelectSDCard();  // Select SD Card on SPI bus
  delay(50);  // Allow power stabilization
  
  if (!SD.begin(SDCARD_CS_PIN)) {
    callback(String(" ") + APP_NAME, "> SDC Error...", "", "");
    Serial.println("SDCard initialization failed!");
    Serial.println("Check: 1) Card inserted 2) Wiring 3) Card format (FAT32)");
    sdCardStatus = false;
  } else {
    callback(String(" ") + APP_NAME, "> SDC Loaded...", "", "");
    Serial.println("SDCard initialization done.");
    
    // Verify card is working by attempting to open root directory
    File root = SD.open("/");
    if(root) {
      Serial.println("SD Card  verified and ready");
      root.close();
      sdCardStatus = true;
    } else {
      Serial.println("SD Card mount failed!");
      sdCardStatus = false;
    }
  }
  
  Config_DeselectAll();  // Deselect after init
}

void SDCard_Save(String filename, String data, bool newLine = true) {
  Config_SelectSDCard();  // Select SD Card
  
  File writeFile = SD.open(filename, FILE_WRITE);

  if (writeFile) {
    if(newLine) {
      writeFile.println(data);
    } else {
      writeFile.print(data);
    }
    writeFile.flush();  // Ensure data is written
    writeFile.close();
    Serial.println("SD: Saved Success - " + filename);
  } else {
    Serial.println("SD: Save Failed - " + filename);
  }
  
  Config_DeselectAll();  // Deselect after operation
}
