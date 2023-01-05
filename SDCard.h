
File writeFile;
File readFile;

void SDCard_Init() {
  if (!SD.begin(2)) {
    Serial.println("SDCard initialization failed!");
  } else {
    Serial.println("SDCard initialization done.");
  }
}

String SDCard_Save(String filename, String data) {
  
  writeFile = SD.open(filename, FILE_WRITE);

  if (writeFile) {
    writeFile.println(data);
    writeFile.close();
    Serial.println("Saving Success!");
  } else {
    Serial.println("Saving Failed!");
  }
}

String SDCard_Load(String filename) {

  String data = "";
  readFile = SD.open(filename);

  if (readFile) {

    // read from the file until there's nothing else in it:
    while (readFile.available()) {
      //Serial.write(myFile.read());
      data = readFile.readString();
    }

    readFile.close(); // close the file:
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  return data;
}