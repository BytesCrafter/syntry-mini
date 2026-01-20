# Syntry Access - AI Coding Guide

## Project Overview
Arduino-based RFID door access control system for ESP8266 with web-based management. Manages physical access via RFID cards with file-based user database on SD card. Features WiFi hotspot for configuration with captive portal web UI, EEPROM-based settings, and comprehensive user management.

## Architecture & Components

### Hardware Integration (ESP8266 NodeMCU)
- **RFID**: MFRC522 on SPI (CS=GPIO0/D3, RST=software) - user authentication
- **SD Card**: SPI-based storage (CS=GPIO2/D4) - user database, powered 5V with 220Ω MISO protection
- **LCD I2C**: 16x2 display at 0x27 (SCL=GPIO5/D1, SDA=GPIO4/D2) - status/feedback messages
- **Outputs**: Relay (GPIO16/D0), Buzzer (GPIO15/D8)
- **Shared SPI Bus**: RFID and SD Card share MOSI/MISO/SCK; explicit CS pin management required

### Module-Based Design
Each `.h` file is a self-contained module with init/loop functions:
- **Config.h**: Global settings, EEPROM management (password/hostname), SPI bus arbitration, serial commands
- **Rfid.h**: Card reading with SPI selection, UID conversion via `Helper_array_to_string()`
- **SDCard.h**: File I/O with SPI protection, user database (`users/{uid}`), settings storage
- **Hotspot.h**: WiFi AP + captive portal (192.168.4.1), all web routes, user management
- **Helper.h**: HTML generation (teal gradient theme), string utilities, web UI pages
- **Display.h**: LCD I2C with communication test, callback pattern `Display_Show(line1, line2)`
- **Buzzer.h/Relay.h**: Hardware output control with feedback patterns

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

3. **Hybrid Storage Architecture**:
   - **EEPROM** (persistent, 512 bytes):
     - Address 0-33: Admin password (max 32 chars + length byte)
     - Address 34-55: Device hostname (max 20 chars + length byte)
   - **SD Card** (file-based):
     - `users/{uid}` - Empty file = authorized user
     - `settings/wifiname` - Saved WiFi SSID
     - `settings/wifipass` - Saved WiFi password

### Web Interface Architecture
Captive portal on WiFi AP (SSID: "Syntry Access - {hostname}"):
- **Routes**: 
  - Auth: `/login`, `/change-password`, `/update-password`
  - Modes: `/menu`, `/access`, `/add`, `/remove`, `/verify`
  - Management: `/manage-users`, `/delete-user`, `/change-hostname`, `/update-hostname`
  - Network: `/wifi-connect`, `/save-wifi`, `/system`
- **HTML Generation**: All UI in Helper.h using minified CSS (~800 chars), teal gradient theme (#1ab394)
  - Grid layout: 2x3 for mode buttons (100px height), 2x1 split for restart/logout
  - CSS classes: `.grid` (2-column), `.split` (50/50), `.c` (centered container 500px max)
  - Button colors: Purple (#8e44ad) for Manage Users, Orange (#f39c12) for System, Red (#e53e3e) for Logout
- **Authentication**: EEPROM-based password (`Config_LoadPassword()`), defaults to "admin"
- **Mode Timeout**: Operations (`add`/`remove`/`verify`) auto-revert to `access` after 5 minutes
- **User Management**: Web UI lists all registered UIDs with individual delete buttons
- **System Info Page**: Displays device name, firmware v1.4.0 + build date, hardware status (✓/✗), flash/heap memory stats, chip ID, WiFi SSID

## Critical Patterns & Conventions

### SPI Device Management (CRITICAL)
**Problem**: RFID and SD Card share SPI bus, causing device malfunctions without proper arbitration.  
**Solution**: Explicit CS pin management functions in Config.h:
```cpp
Config_SelectRFID();      // Before any RFID operation
Config_SelectSDCard();    // Before any SD card operation
Config_DeselectAll();     // After completing operations
```
**Pattern**: Always wrap SD/RFID operations:
```cpp
Config_SelectSDCard();
File f = SD.open("users/12345678");
if(f) { f.close(); }
Config_DeselectAll();
```
**Boot Safety**: CS pins initialized HIGH in `Config_Init()` BEFORE `SPI.begin()`

### UID Formatting
RFID cards return byte arrays; convert to hex strings:
```cpp
char str[32] = "";
Helper_array_to_string(mfrc522.uid.uidByte, 4, str);  // Outputs: "AABBCCDD"
```

### Special Admin Reset
Tapping the admin card (UID = hostname value from EEPROM) resets EEPROM password to "admin".

### Hardware Feedback Protocol
Every operation has audio-visual feedback using `APP_NAME` constant:
```cpp
Display_Show(String(" ") + APP_NAME, " ACCESS GRANTED");
Buzzer_Play(1, 900, 50);  // (count, frequency, duration)
```
**String Concatenation Rule**: Wrap first string literal in `String()` constructor when concatenating with `APP_NAME`:
```cpp
// CORRECT:
ptr += String("<h1>") + APP_NAME + "</h1>";
Display_Show(String(" ") + APP_NAME, "MESSAGE");

// WRONG (compilation error):
ptr += "<h1>" + APP_NAME + "</h1>";  // Can't concat two C-string literals
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

### Testing Workflow
**Recommended sequence for validating hardware:**
1. **Serial Monitor First** (9600 baud) - Check initialization logs for all modules
2. **LCD Display** - Should show "{APP_NAME}" + "by BytesCrafter" at boot
3. **SPI Bus Test** - Verify both RFID and SD Card initialize without conflicts (status variables: `rfidStatus`, `sdCardStatus`)
4. **SD Card Structure** - Manually create `/users` and `/settings` folders if auto-create fails
5. **RFID Read** - Tap a test card, should see UID in serial (8-char hex)
6. **WiFi AP** - Connect to "Syntry Access - {hostname}", access captive portal at 192.168.4.1
7. **Web Operations** - Login (admin/admin), test Add Card mode, verify user appears in Manage Users
8. **Hardware Outputs** - Test relay activation and buzzer feedback patterns

**Troubleshooting checklist:**
- No RFID reads → Check SPI selection calls, verify CS pins HIGH during boot
- SD card fails → Verify 220Ω resistor on MISO, check FAT32 format, test root directory access
- Device reboots randomly → Likely SPI conflicts, ensure `Config_DeselectAll()` after operations
- Web UI slow → HTML strings too large, check Helper.h minification (~800 char target)

## Common Pitfalls & Critical Issues

- **SPI Bus Conflicts**: MUST use `Config_SelectSDCard()`/`Config_SelectRFID()`/`Config_DeselectAll()` or device will malfunction
- **Pin Restrictions**: GPIO3 (TX) and GPIO1 (RX) avoided - UART critical, relay on GPIO16 (D0)
- **Boot Pins**: GPIO0 must be HIGH, GPIO15 must be LOW at boot - affects hardware design
- **String Concatenation**: Can't use `+` operator directly on two string literals - wrap first in `String()`
- **5V Hardware**: SD card powered at 5V requires 220Ω resistor on MISO line for ESP8266 protection
- **Memory**: ESP8266 limited RAM - HTML strings minified to ~800 chars, use `String()` sparingly
- **SD Card Safety**: Always `flush()` then `close()`, check file handles, use SPI selection
- **File System**: ESP8266 SD library differs from standard Arduino - no `CARD_NONE` constants, use root directory test
- **EEPROM**: Call `EEPROM.commit()` after writes or changes won't persist

## Key Files & Recent Changes

- **Syntry.ino**: Main loop, mode dispatcher (`catch_Rfid()`), all access control logic
- **Config.h**: Pin defs, EEPROM management, **SPI bus arbitration functions** (v1.4.0)
- **Hotspot.h**: Complete web server with 15+ routes, user management, hostname config
- **Helper.h**: HTML generation (teal theme), includes `Helper_Hotspot_ManageUsers()` for web-based user list
- **SDCard.h**: Enhanced error handling, SPI protection, voltage warnings (v0.2.0)
- **Rfid.h**: Software reset (GPIO16 freed for relay), SPI-aware initialization

**Recent Fixes (Jan 2026)**:
- Moved relay from GPIO3 (TX) to GPIO16 (D0) - eliminated UART interference
- Migrated password/hostname from SD card to EEPROM - improved reliability
- Added comprehensive SPI bus management - fixed device malfunctions
- Implemented user management web UI - list all cards with delete buttons
- Fixed string concatenation with `APP_NAME` constant - compilation errors resolved

## Future Enhancement Opportunities

**High Priority (Planned Features):**
- **Access Logging with Timestamps**: 
  - Implement timestamped logs using NTP client (already initialized in code: `timeClient.begin()`)
  - Store in rotating log file `/logs/access.txt` (last 100 entries to save SD space)
  - Display format: `2026-01-20 14:32:15 | UID:A1B2C3D4 | GRANTED`
  - Add `/logs` route to view recent access history in web UI

- **User Nicknames/Labels**:
  - Store friendly names in `users/{uid}` file content (currently empty files)
  - Update user management UI to show: "John Doe (A1B2C3D4)" with edit button
  - Display name on LCD when card tapped: "WELCOME JOHN" instead of UID
  - Add `/edit-user` route with form: UID (readonly) + Name (input)

- **Access Statistics Dashboard**:
  - Add to system info page: total users, total access attempts today/week
  - Track failed attempts counter (security audit)
  - Display last access time/user
  - Store simple counters in `settings/stats` file

**Medium Priority:**
- **User Count Badge**: Show registered count in menu: "Manage Users (12)"
- **Bulk Operations**: Delete all users button, export/import user list as CSV
- **Uptime Display**: Show last reboot time and system uptime on system info
- **Card Expiry Dates**: Store expiration in user file for temporary access

**Future Considerations:**
- **WiFi Station Mode Integration**: Full dual-mode (AP+STA) or mode switching
- **LED Status Indicators**: Implement LIGHT_RED/GREEN visual feedback
- **Remote API**: WebRequest.h integration for cloud logging/monitoring
