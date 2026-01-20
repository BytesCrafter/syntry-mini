# Syntry Mini v1 - AI Coding Guide

## Project Overview
Arduino-based RFID access control system for ESP8266 with web-based configuration. Manages physical access via RFID cards with file-based user database on SD card. Operates in dual modes: WiFi hotspot for configuration and WiFi client for network connectivity.

## Architecture & Components

### Hardware Integration (ESP8266 NodeMCU)
- **RFID**: MFRC522 on SPI (CS=D3/GPIO2, RST=D0/GPIO16) - user authentication
- **SD Card**: SPI-based storage (CS=D4/GPIO2) - user database and settings
- **LCD I2C**: 16x2 display (SCL=D1, SDA=D2) - status/feedback messages
- **Outputs**: Relay (RX/GPIO3), RGB LEDs (SD1/SD2), Buzzer (D8/GPIO15)
- **Shared SPI Bus**: RFID and SD Card share MOSI/MISO/SCK; CS pins for device selection

### Module-Based Design
Each `.h` file is a self-contained module with init/loop functions:
- **Config.h**: Global settings, serial commands (`goto access`, `goto sethostname`, `reset`)
- **Rfid.h**: Card reading, UID conversion via `Helper_array_to_string()`
- **SDCard.h**: File I/O for users (`users/{uid}`) and settings (`settings/hostname`)
- **Hotspot.h**: WiFi AP + captive portal (192.168.4.1) with ESP8266WebServer
- **WifiClient.h**: WiFi station mode, loads credentials from SD card
- **Display.h**: LCD messages via callback pattern `Display_Show(line1, line2)`
- **Helper.h**: HTML generation for web UI, string utilities, authentication pages

### Data Flow & State Management
1. **Mode System**: Global `rfidMode` variable controls RFID behavior:
   - `access`: Normal operation - check user in `users/{uid}` file
   - `add`: Register new cards - create `users/{uid}` file
   - `remove`: Delete cards - remove `users/{uid}` file
   - `verify`: Check card existence without access
   - `sethostname`: Set admin card ID in `settings/hostname`

2. **Callback Pattern**: Modules use function pointers for loose coupling:
   ```cpp
   Rfid_Listen(&catch_Rfid);  // RFID events
   SDCard_Init(&Display_Show);  // Status messages
   ```

3. **File-Based Database**: SD card structure:
   ```
   users/
     {uid}          - Empty file = authorized user
     admin          - Admin password (plaintext)
   settings/
     hostname       - Unique device identifier (10-digit random)
     wifiname       - Saved WiFi SSID
     wifipass       - Saved WiFi password
   ```

### Web Interface Architecture
Captive portal on WiFi AP (SSID: "Syntry Mini - {hostname}"):
- **Routes**: `/login`, `/menu`, `/access`, `/add`, `/remove`, `/verify`, `/change-password`, `/wifi-connect`
- **HTML Generation**: All UI in Helper.h functions (`Helper_Hotspot_To_Menu()`, etc.)
- **Authentication**: Compare against `users/admin` file, defaults to "admin"/"admin"
- **Mode Timeout**: Operations (`add`/`remove`/`verify`) auto-revert to `access` after 5 minutes

## Critical Patterns & Conventions

### SPI Device Management
**Problem**: RFID and SD Card share SPI bus, causing conflicts.  
**Solution**: Explicit CS pin control (though not fully implemented):
```cpp
// Intended pattern (needs completion):
digitalWrite(SDCARD_CS_PIN, HIGH);  // Deselect SD
digitalWrite(RFID_CS_PIN, LOW);     // Select RFID
// ... RFID operation ...
digitalWrite(RFID_CS_PIN, HIGH);    // Deselect RFID
```

### UID Formatting
RFID cards return byte arrays; convert to hex strings:
```cpp
char str[32] = "";
Helper_array_to_string(mfrc522.uid.uidByte, 4, str);  // Outputs: "AABBCCDD"
```

### Special Admin Reset
Tapping the admin card (UID = hostname value) deletes `users/admin` and resets password.

### Hardware Feedback Protocol
Every operation has audio-visual feedback:
- Success: 900Hz beep + "ACCESS GRANTED" message
- Failure: 300-400Hz beep + "ACCESS DENIED" message
- Configuration: 700Hz beep when entering modes

### Display Update Pattern
Always pair display messages with buzzer feedback:
```cpp
Display_Show(" Syntry Mini v1", " ACCESS GRANTED");
Buzzer_Play(1, 900, 50);  // (count, frequency, duration)
```

## Development Workflow

### Required Libraries
Install these libraries before compiling (via Arduino Library Manager or GitHub):

**Core Libraries:**
- **MFRC522** - RFID reader interface  
  Install: Library Manager → "MFRC522 by GithubCommunity"  
  GitHub: https://github.com/miguelbalboa/rfid

- **LiquidCrystal_I2C** - LCD display driver  
  Install: Library Manager → "LiquidCrystal I2C by Frank de Brabander"  
  GitHub: https://github.com/johnrickman/LiquidCrystal_I2C

- **NTPClient** - Network time synchronization  
  Install: Library Manager → "NTPClient by Fabrice Weinberg"  
  GitHub: https://github.com/arduino-libraries/NTPClient

**Optional Libraries:**
- **Rtc by Makuna** - Real-time clock support (for future logging features)  
  GitHub: https://github.com/Makuna/Rtc/wiki

- **Adafruit Unified Sensor** - Sensor abstraction layer  
  GitHub: https://github.com/adafruit/Adafruit_Sensor

**Built-in** (included with ESP8266 core): ESP8266WiFi, ESP8266WebServer, DNSServer, SPI, Wire, SD, EEPROM

### Build & Upload
Use Arduino IDE with ESP8266 board package:
1. Install ESP8266 board: File > Preferences > Additional Board Manager URLs:  
   `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
2. Select: Tools > Board > NodeMCU 1.0 (ESP-12E Module)
3. Port: Match your USB serial port
4. Upload Speed: 115200 (stable for most boards)

### Serial Debugging
Connect at 9600 baud. Available commands:
```
goto access       - Normal access mode
goto sethostname  - Set admin card
reset            - Full device reset
restart          - Soft restart
```

### SD Card Setup
Format FAT32, create folder structure manually or let device auto-create:
```
/users/
/settings/
```

### Testing Modes
1. Flash firmware
2. Connect to "Syntry Mini - {random}" WiFi AP
3. Navigate to any URL (captive portal redirects)
4. Login: admin/admin (default)
5. Use web UI to switch modes, tap test cards

## Common Pitfalls

- **Memory**: ESP8266 has limited RAM. HTML strings in Helper.h are concatenated; avoid large static strings.
- **String Leaks**: Arduino String objects can fragment memory. Consider using `char[]` for long operations.
- **Timing**: `delay()` blocks main loop; affects RFID responsiveness. Keep delays minimal.
- **SD Card Corruption**: Always close files after operations (`file.close()`).
- **WiFi Mode Conflicts**: Can't run AP and Station simultaneously reliably. Current design prioritizes AP mode.

## Key Files Reference

- [Syntry.ino](Syntry.ino): Main loop, mode dispatcher (`catch_Rfid()`)
- [Hotspot.h](Hotspot.h): Complete web server implementation
- [Helper.h](Helper.h): All HTML templates and authentication UI
- [Config.h](Config.h): Pin definitions, mode state, serial commands
- [Rfid.h](Rfid.h): MFRC522 interface, UID reading

## Missing/TODO Features

- Logging: TODOs in code for timestamped access logs
- NTP Client: Commented out in main loop (timeClient)
- WiFi Station Mode: Logic exists but not fully integrated
- LED Indicators: Defined pins (LIGHT_RED/GREEN) but no implementation
- Error Recovery: Limited retry/fallback for hardware failures
