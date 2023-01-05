//MFRC
#include <MFRC522.h>

//MFRC
#define RST_PIN 16          // Configurable, see typical pin layout above /def 5
#define SS_PIN 0        // Configurable, see typical pin layout above /def 4
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

//MFRC
void Rfid_Init() {
  mfrc522.PCD_Init();		// Init MFRC522
  delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void Rfid_Listen(bool (*callback)(String)) {
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

  //Set indicator to Busy
  // digitalWrite(RED, HIGH);
  // digitalWrite(GREEN, LOW);
  // sendTune(1, 800, 100);

  //Show on LCD UID
  char str[32] = "";
  Helper_array_to_string(mfrc522.uid.uidByte, 4, str); //Insert (byte array, length, char array for output)
  //lcd.print(str);

  // DEBUG: Dump debug info about the card; PICC_HaltA() is automatically called
	//mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  callback( str );
}