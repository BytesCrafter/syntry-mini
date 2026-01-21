/*
  Module: MFRC522 Module.
  Version: 0.2.0
  
  RECOVERY SYSTEM: Handles power interruptions, electrical noise, and wire disconnects.
  Uses UINT8_MAX for RST_PIN to indicate no hardware reset connection (software-only reset).
*/

//Declare libraries
#include <MFRC522.h>

// RST_PIN = UINT8_MAX means NO hardware reset pin connected (library will use soft reset only)
// This is the CORRECT way per MFRC522 library documentation
#define RST_PIN UINT8_MAX
MFRC522 mfrc522(RFID_CS_PIN, RST_PIN); // Create MFRC522 instance

// Self-recovery tracking (optimized intervals for performance)
unsigned long lastRfidHealthCheck = 0;
const unsigned long RFID_HEALTH_CHECK_INTERVAL = 10000; // Check every 10 seconds if issues detected
const unsigned long RFID_PROACTIVE_CHECK_INTERVAL = 30000; // Proactive check every 60 seconds when OK
int consecutiveRfidFailures = 0;
const int RFID_MAX_FAILURES_BEFORE_RESET = 3; // Reset after 3 consecutive failures

// Debug: Track listen loop iterations (reduced frequency)
unsigned long lastRfidDebugLog = 0;
const unsigned long RFID_DEBUG_LOG_INTERVAL = 60000; // Log status every 2 minutes

// Proactive recovery timer
unsigned long lastProactiveCheck = 0;

// Track if we need full re-initialization (after power cycle)
bool rfidNeedsFullInit = false;

//Initialize RFID module.
void Rfid_Init(void (*callback)(String, String, String, String)) {
  Serial.println("[RFID Rfid_Init] Starting RFID initialization...");
  
  Config_SelectRFID();  // Select RFID on SPI bus
  
  // Use library's PCD_Init which handles RST_PIN=UINT8_MAX correctly
  // When RST_PIN=UINT8_MAX, library uses software reset only
  mfrc522.PCD_Init();
  delay(500);  // Some boards need extra time after init

  // Verify SPI communication first
  byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.println("[RFID Rfid_Init] Version register: 0x" + String(version, HEX));
  
  if (version == 0x00 || version == 0xFF) {
    // SPI communication failure
    Config_AddBootLog("MFRC522: No SPI response");
    Serial.println("[RFID Rfid_Init] ERROR: No SPI communication!");
    Config_DeselectAll();
    callback(String(" ") + APP_NAME, "> RFID Error...", "", "");
    rfidStatus = false;
    return;
  }
  
  // Set maximum antenna gain for better card detection range
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  delay(10);
  
  // Ensure antenna is on
  mfrc522.PCD_AntennaOn();
  delay(50);
  
  // Verify antenna is actually ON
  byte txControl = mfrc522.PCD_ReadRegister(mfrc522.TxControlReg);
  if ((txControl & 0x03) != 0x03) {
    Serial.println("[RFID Rfid_Init] Antenna not on, forcing...");
    mfrc522.PCD_WriteRegister(mfrc522.TxControlReg, 0x83);
    delay(50);
    txControl = mfrc522.PCD_ReadRegister(mfrc522.TxControlReg);
  }
  Serial.println("[RFID Rfid_Init] TxControl: 0x" + String(txControl, HEX) + " (Antenna: " + String((txControl & 0x03) == 0x03 ? "ON" : "OFF") + ")");
  
  // Report version
  Config_AddBootLog("[RFID Rfid_Init] v0x" + String(version, HEX));
  Serial.println("[RFID Rfid_Init] MFRC522 Firmware: 0x" + String(version, HEX));
  
  // Check if version is known good
  if (version == 0x91 || version == 0x92) {
    Config_AddBootLog("[RFID Rfid_Init] OK");
    callback(String(" ") + APP_NAME, "> RFID Loaded...", "", "");
    rfidStatus = true;
  } else if (version == 0x88) {
    // FM17522 clone - should work
    Config_AddBootLog("[RFID Rfid_Init] Clone detected");
    callback(String(" ") + APP_NAME, "> RFID Clone...", "", "");
    rfidStatus = true;
  } else {
    // Unknown but responding - try anyway
    Config_AddBootLog("[RFID Rfid_Init] Unknown version");
    callback(String(" ") + APP_NAME, "> RFID Unknown...", "", "");
    rfidStatus = true;
  }
  
  Config_DeselectAll();
  Serial.println("[RFID Rfid_Init] Initialization complete. Status: " + String(rfidStatus ? "OK" : "FAILED"));
  Serial.println();
}

// Self-test and recovery function
// Returns true if RFID module is responding and ready
bool Rfid_HealthCheck() {
  Config_SelectRFID();
  
  // Read version register to verify SPI communication
  byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  
  // 0x00 or 0xFF = no SPI response (module disconnected or not powered)
  if (version == 0x00 || version == 0xFF) {
    Config_DeselectAll();
    Serial.println("[RFID HealthCheck] FAIL - No SPI response (0x" + String(version, HEX) + ")");
    return false;
  }
  
  // Check known good versions
  if (version != 0x91 && version != 0x92 && version != 0x88) {
    Config_DeselectAll();
    Serial.println("[RFID HealthCheck] WARN - Unknown version: 0x" + String(version, HEX));
    // Continue anyway, might still work
  }
  
  // Verify antenna is ON (TxControlReg bits 0-1 must be set)
  byte txControl = mfrc522.PCD_ReadRegister(mfrc522.TxControlReg);
  bool antennaOn = (txControl & 0x03) == 0x03;
  
  if (!antennaOn) {
    Serial.println("[RFID HealthCheck] FAIL - Antenna OFF (TxControl: 0x" + String(txControl, HEX) + ")");
    Config_DeselectAll();
    return false;
  }
  
  // Check ModWidthReg (should be 0x26 after proper init)
  byte modWidth = mfrc522.PCD_ReadRegister(mfrc522.ModWidthReg);
  if (modWidth == 0x00 || modWidth == 0xFF) {
    Serial.println("[RFID HealthCheck] FAIL - ModWidth invalid (0x" + String(modWidth, HEX) + ")");
    Config_DeselectAll();
    return false;
  }
  
  Config_DeselectAll();
  return true;
}

// Perform complete RFID module reinitialization
// This mimics what happens at boot time
void Rfid_FullInit() {
  Serial.println("[RFID FullInit] Starting complete reinitialization...");
  
  Config_SelectRFID();
  
  // Step 1: Software reset using library's PCD_Reset
  Serial.println("[RFID FullInit] Step 1: Software reset...");
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, MFRC522::PCD_SoftReset);
  
  // Wait for reset to complete (PowerDown bit to clear)
  delay(100);
  unsigned long timeout = millis() + 1000;
  while (millis() < timeout) {
    byte cmd = mfrc522.PCD_ReadRegister(mfrc522.CommandReg);
    if (!(cmd & (1 << 4))) break; // PowerDown bit cleared
    delay(50);
  }
  delay(500);
  
  // Step 2: Reset communication registers (per library PCD_Init)
  Serial.println("[RFID FullInit] Step 2: Reset registers...");
  mfrc522.PCD_WriteRegister(mfrc522.TxModeReg, 0x00);
  mfrc522.PCD_WriteRegister(mfrc522.RxModeReg, 0x00);
  mfrc522.PCD_WriteRegister(mfrc522.ModWidthReg, 0x26);
  
  // Step 3: Configure timer (per library PCD_Init)
  Serial.println("[RFID FullInit] Step 3: Configure timer...");
  mfrc522.PCD_WriteRegister(mfrc522.TModeReg, 0x80);      // TAuto=1
  mfrc522.PCD_WriteRegister(mfrc522.TPrescalerReg, 0xA9); // Prescaler
  mfrc522.PCD_WriteRegister(mfrc522.TReloadRegH, 0x03);   // Reload high
  mfrc522.PCD_WriteRegister(mfrc522.TReloadRegL, 0xE8);   // Reload low (25ms timeout)
  
  // Step 4: Configure modulation and CRC
  Serial.println("[RFID FullInit] Step 4: Configure modulation...");
  mfrc522.PCD_WriteRegister(mfrc522.TxASKReg, 0x40);      // 100% ASK modulation
  mfrc522.PCD_WriteRegister(mfrc522.ModeReg, 0x3D);       // CRC preset 0x6363
  
  // Step 5: Enable antenna with maximum gain
  Serial.println("[RFID FullInit] Step 5: Enable antenna...");
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  delay(10);
  mfrc522.PCD_AntennaOn();
  delay(50);
  
  // Verify antenna enabled
  byte txControl = mfrc522.PCD_ReadRegister(mfrc522.TxControlReg);
  if ((txControl & 0x03) != 0x03) {
    Serial.println("[RFID FullInit] Antenna not on, forcing...");
    mfrc522.PCD_WriteRegister(mfrc522.TxControlReg, 0x83);
    delay(10);
  }
  
  Config_DeselectAll();
  delay(100);
  
  // Verify initialization success
  if (Rfid_HealthCheck()) {
    Serial.println("[RFID FullInit] SUCCESS - Module ready");
    
    // Dump all critical registers for debugging
    Config_SelectRFID();
    Serial.println("[RFID FullInit] Register dump:");
    Serial.println("  Version:    0x" + String(mfrc522.PCD_ReadRegister(mfrc522.VersionReg), HEX));
    Serial.println("  TxControl:  0x" + String(mfrc522.PCD_ReadRegister(mfrc522.TxControlReg), HEX));
    Serial.println("  ModWidth:   0x" + String(mfrc522.PCD_ReadRegister(mfrc522.ModWidthReg), HEX));
    Serial.println("  TxMode:     0x" + String(mfrc522.PCD_ReadRegister(mfrc522.TxModeReg), HEX));
    Serial.println("  RxMode:     0x" + String(mfrc522.PCD_ReadRegister(mfrc522.RxModeReg), HEX));
    Serial.println("  TxASK:      0x" + String(mfrc522.PCD_ReadRegister(mfrc522.TxASKReg), HEX));
    Serial.println("  Mode:       0x" + String(mfrc522.PCD_ReadRegister(mfrc522.ModeReg), HEX));
    Config_DeselectAll();
  } else {
    Serial.println("[RFID FullInit] FAILED - Health check did not pass");
  }
}

// Recovery procedure using CS pin isolation
// This simulates a power cycle when no RST pin is connected
void Rfid_Recover() {
  Serial.println("[RFID Recover] Starting recovery with CS isolation...");
  
  bool recovered = false;
  
  for (int attempt = 0; attempt < 3 && !recovered; attempt++) {
    Serial.println("[RFID Recover] Attempt " + String(attempt + 1) + "/3");
    
    // STEP 1: Isolate module from SPI bus
    Serial.println("[RFID Recover] Step 1: Isolating from SPI bus...");
    Config_DeselectAll();
    digitalWrite(RFID_CS_PIN, HIGH);
    pinMode(RFID_CS_PIN, INPUT);      // High-impedance = electrically disconnected
    delay(100 + (attempt * 100));     // Progressive delay
    
    // STEP 2: Reconnect module
    Serial.println("[RFID Recover] Step 2: Reconnecting...");
    pinMode(RFID_CS_PIN, OUTPUT);
    digitalWrite(RFID_CS_PIN, HIGH);  // Start deselected
    delay(50);
    
    // STEP 3: Full reinitialization
    Rfid_FullInit();
    
    // STEP 4: Verify recovery
    delay(100);
    if (Rfid_HealthCheck()) {
      recovered = true;
      Serial.println("[RFID Recover] SUCCESS on attempt " + String(attempt + 1));
    } else {
      Serial.println("[RFID Recover] Attempt " + String(attempt + 1) + " failed");
      delay(200 * (attempt + 1));
    }
  }
  
  if (recovered) {
    rfidStatus = true;
    consecutiveRfidFailures = 0;
    rfidNeedsFullInit = false;
  } else {
    Serial.println("[RFID Recover] FAILED after 3 attempts - module may be disconnected");
    rfidStatus = false;
  }
}

//Called in loop to listen for RFID cards (OPTIMIZED)
void Rfid_Listen(bool (*callback)(String)) {  
  // === RECOVERY MODE: If RFID is down, attempt recovery periodically ===
  if (!rfidStatus) {
    if (millis() - lastRfidHealthCheck > RFID_HEALTH_CHECK_INTERVAL) {
      lastRfidHealthCheck = millis();
      Serial.println(F("[RFID] Attempting recovery..."));
      Rfid_Recover();
    }
    return;  // Don't call callback with "null" every loop - wastes time
  }
  
  // === PERIODIC HEALTH CHECK: Less frequent to improve performance ===
  if (millis() - lastProactiveCheck > RFID_PROACTIVE_CHECK_INTERVAL) {
    lastProactiveCheck = millis();
    rfidStatus = false;
    Rfid_Recover();
    return;
  }
  
  // === SELECT RFID MODULE ===
  Config_SelectRFID();
  
  // === CARD DETECTION (fast path - no extra checks unless card present) ===
  if (!mfrc522.PICC_IsNewCardPresent()) {
    Config_DeselectAll();
    return;  // No card - exit fast
  }

  // === READ CARD UID ===
  if (!mfrc522.PICC_ReadCardSerial()) {
    Config_DeselectAll();
    return;  // Card read failed - exit
  }

  // Card successfully read - convert UID to hex string
  char str[32] = "";
  Helper_array_to_string(mfrc522.uid.uidByte, 4, str);
  
  Serial.println("[RFID] UID: " + String(str));
  
  // Halt card to prepare for next read
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  Config_DeselectAll();
  callback(str);
}