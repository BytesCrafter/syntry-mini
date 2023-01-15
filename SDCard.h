/*
  Module: SDCard Module.
  Version: 0.1.0
*/

//Initialize the SD Card.
void SDCard_Init() {
  if (!SD.begin(SDCARD_PIN)) {
    Serial.println("SDCard initialization failed!");
  } else {
    Serial.println("SDCard initialization done.");
  }
}

// String SDCard_Save(String filename, String data, bool overwrite = false) {
  
//   File writeFile = SD.open(filename, FILE_WRITE);

//   if (writeFile) {
//     if(overwrite) {
//       writeFile.print(data);
//     } else {
//       writeFile.println(data);
//     }
    
//     writeFile.close();
//     Serial.println("SD: Saved Success!");
//   } else {
//     Serial.println("SD: Save Failed!");
//   }
// }

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