/*
  Module: SDCard Module.
  Version: 0.1.0
*/

//Initialize the SD Card.
void SDCard_Init(void (*callback)(String, String, String, String)) {
  Config_SelectSDCard();  // Select SD Card on SPI bus
  
  if (!SD.begin(SDCARD_CS_PIN)) {
    callback(" Syntry Mini v1", "> SDC Error...", "", "");
    Serial.println("SDCard initialization failed!");
    sdCardStatus = false;
  } else {
    callback(" Syntry Mini v1", "> SDC Loaded...", "", "");
    Serial.println("SDCard initialization done.");
    sdCardStatus = true;
  }
  
  Config_DeselectAll();  // Deselect after init
}

void SDCard_Save(String filename, String data, bool newLine = true) {
  
  File writeFile = SD.open(filename, FILE_WRITE);

  if (writeFile) {
    if(newLine) {
      writeFile.println(data);
    } else {
      writeFile.print(data);
    }
    
    writeFile.close();
    Serial.println("SD: Saved Success!");
  } else {
    Serial.println("SD: Save Failed!");
  }
}

// String SDCard_Load(String filename) {

//   String data = "";
//   File readFile = SD.open(filename);

//   if (readFile) {
//     // read from the file until there's nothing else in it:
//     while (readFile.available()) {
//       //Serial.write(myFile.read());
//       data = readFile.readString();
//     }

//     readFile.close(); // close the file:
//     Serial.println("SD: Read Success!");
//   } else {
//     // if the file didn't open, print an error:
//     Serial.println("SD: Read Failed!");
//   }

//   return data;
// }

// bool SDCard_Delete(String filename) {

//   SD.remove(filename);

//   if (SD.exists(filename)) {
//     return false;
//   }

//   return true;
// }