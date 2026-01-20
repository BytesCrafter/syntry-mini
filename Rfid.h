/*
  Module: MFRC522 Module.
  Version: 0.1.0
*/

//Declare libraries
#include <MFRC522.h>

//Set the target pin - Using software reset instead of hardware RST
#define RST_PIN 0 // Not used - using soft reset Prev: GPIO16 = D0
MFRC522 mfrc522(RFID_CS_PIN, RST_PIN); // Create MFRC522 instance

//Initilialize RFID module.
void Rfid_Init(void (*callback)(String, String, String, String)) {
  Config_SelectRFID();  // Select RFID on SPI bus
  
  mfrc522.PCD_Init();		// Init MFRC522
  delay(500);				// Optional delay. Some board do need more time after init to be ready, see Readme

  Serial.println(F("*****************************"));
  Serial.println(F("MFRC522 Digital self test"));
  Serial.println(F("*****************************"));
  mfrc522.PCD_DumpVersionToSerial();  // Show version of PCD - MFRC522 Card Reader
  Serial.println(F("-----------------------------"));
  Serial.println(F("Only known versions supported"));
  Serial.println(F("-----------------------------"));
  
  Serial.println(F("Performing test..."));
  bool result = mfrc522.PCD_PerformSelfTest(); // perform the test
  Serial.println(F("-----------------------------"));
  Serial.print(F("Result: "));
  if (result) {
    Serial.println(F("OK"));
    callback(String(" ") + APP_NAME, "> RFID Loaded...", "", "");
    rfidStatus = true;
  } else {
    Serial.println(F("DEFECT or UNKNOWN"));
    callback(String(" ") + APP_NAME, "> RFID Error...", "", "");
  }
  Serial.println();
  
  //Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

//Called in loop to listen.
void Rfid_Listen(bool (*callback)(String)) {
  Config_SelectRFID();  // Select RFID before reading
  
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
    callback("null");
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
    callback("null");
		return;
	}

  //Insert (byte array, length, char array for output)
  char str[32] = "";
  Helper_array_to_string(mfrc522.uid.uidByte, 4, str);

  // DEBUG: Dump debug info about the card; PICC_HaltA() is automatically called
	//mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  Config_DeselectAll();  // Deselect all after reading
  callback( str );
}