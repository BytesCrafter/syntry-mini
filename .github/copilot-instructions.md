# Syntry Access - AI Coding Guide

## Project Overview
Arduino-based RFID door access control system for ESP8266 NodeMCU with web-based management. Manages physical access via RFID cards with file-based user database on SD card. Features WiFi hotspot for configuration with captive portal web UI, EEPROM-based settings, comprehensive user management with nicknames and pagination, and optional WiFi client mode.

**Current Version**: 1.5.0 (Build: 2026-01-22)  
**Platform**: ESP8266 NodeMCU (ESP-12E Module)  
**Serial Baud**: 9600

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
- **Rfid.h**: Card reading with SPI selection, UID conversion, **self-recovery system** for power interruptions
- **SDCard.h**: File I/O with SPI protection, user database (`users/{uid}`), settings storage
- **Hotspot.h**: WiFi AP + captive portal (192.168.4.1), 25+ web routes, user management
- **Helper.h**: HTML generation (teal gradient theme), string utilities, 14 page builder functions
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

### User Database Architecture
**File Structure**: `users/{uid}` where UID is 8-16 char hex string
- **Empty file** = authorized user (legacy)
- **File content** = nickname (max 20 chars, displayed in web UI and on LCD)

**User Management Features** (Hotspot.h + Helper.h):
- Pagination: 10 users per page with Previous/Next navigation
- Nicknames: Stored as file content, editable via `/edit-user` route
- Manual Add: `/add-user` route with UID validation (hex chars only)
- Bulk Delete: `/clear-all-users` with admin password confirmation
- Memory-efficient: Uses `yield()` every 20 files to prevent watchdog reset

### Hybrid Storage Architecture
**EEPROM** (persistent, 512 bytes) - Critical settings:
- Address 0-33: Admin password (max 32 chars + length byte, default "admin")
- Address 34-55: Device hostname (max 20 chars + length byte)
- Address 56-120: WiFi SSID (max 64 chars + length byte)
- Address 121-185: WiFi Password (max 64 chars, **optional for open networks**)
- Address 186: WiFi Auto-Connect flag (1 byte)

**EEPROM API Pattern**:
```cpp
Config_SavePassword(String) / Config_LoadPassword() -> String
Config_SaveHostname(String) / Config_LoadHostname() -> String  
Config_SaveWifiSSID(String) / Config_LoadWifiSSID() -> String
Config_SaveWifiPassword(String) / Config_LoadWifiPassword() -> String
Config_SaveWifiAuto(bool) / Config_LoadWifiAuto() -> bool
```
⚠️ **Critical**: Always call `EEPROM.commit()` after writes!

### Web Interface Architecture
Captive portal on WiFi AP (SSID: "Syntry Access - {hostname}"):

**Session Management**: 
- URL-based token authentication (captive portal compatible, no cookies)
- 30-minute timeout (`SESSION_TIMEOUT` constant)
- Pattern: `Hotspot_IsSessionValid()` → `Hotspot_RequireAuth()` → `Hotspot_AddToken(url)`

**Routes** (25+ endpoints):
- Auth: `/login`, `/logout`, `/change-password`, `/update-password`
- Modes: `/menu`, `/access`, `/add`, `/remove`, `/verify`
- User Management: `/manage-users`, `/add-user`, `/create-user`, `/edit-user`, `/save-user`, `/delete-user`, `/clear-all-users`
- Settings: `/change-hostname`, `/update-hostname`, `/boot-logs`, `/system`
- Network: `/wifi-connect`, `/save-wifi`, `/wifi-manual-connect`, `/wifi-disconnect`, `/wifi-toggle-auto`

**HTML Generation Pattern** (Helper.h):
```cpp
String Helper_Hotspot_PageName(String message = "", int page = 0) {
  extern String activeSessionToken;
  String token = "<input type='hidden' name='token' value='" + activeSessionToken + "'>";
  String ptr = Helper_HttpHeader();
  // ... build page content with ptr += ...
  ptr += Helper_HttpBackToMenu();
  ptr += Helper_HttpFooter();
  return ptr;
}
```

## Critical Patterns & Conventions

### SPI Device Management (CRITICAL)
**Problem**: RFID and SD Card share SPI bus, causing malfunctions without proper arbitration.
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

### UID Formatting
```cpp
char str[32] = "";
Helper_array_to_string(mfrc522.uid.uidByte, 4, str);  // Outputs: "AABBCCDD"
```

### String Concatenation Rule
```cpp
// CORRECT - wrap first literal in String():
ptr += String("<h1>") + APP_NAME + "</h1>";
Display_Show(String(" ") + APP_NAME, "MESSAGE");

// WRONG - compilation error:
ptr += "<h1>" + APP_NAME + "</h1>";
```

### HTTP Redirect Pattern
Use 302 status code for form submissions:
```cpp
webServer.sendHeader("Location", Hotspot_AddToken("/manage-users?status=Success!&page=" + String(page)), true);
webServer.send(302, "text/plain", "");
```

### Memory-Efficient File Iteration
```cpp
while(entry) {
  // process entry...
  entry.close();
  entry = usersDir.openNextFile();
  if(totalCount % 20 == 0) yield();  // Prevent watchdog reset
}
```

### RFID Self-Recovery System (Rfid.h v0.2.0)
```cpp
#define RST_PIN UINT8_MAX  // No hardware reset pin (GPIO0 conflicts with boot!)
```
Key functions: `Rfid_HealthCheck()`, `Rfid_FullInit()`, `Rfid_Recover()`

## Development Workflow

### Required Libraries
- **MFRC522** - "MFRC522 by GithubCommunity"
- **LiquidCrystal_I2C** - "LiquidCrystal I2C by Frank de Brabander"
- **NTPClient** - "NTPClient by Fabrice Weinberg"

### Build & Upload
1. ESP8266 board URL: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
2. Board: NodeMCU 1.0 (ESP-12E Module)
3. Upload Speed: 115200

### Serial Commands (9600 baud)
```
goto access       - Normal access mode
goto sethostname  - Set admin card
reset            - Full device reset
restart          - Soft restart
```

## Common Pitfalls
- **SPI Conflicts**: MUST use `Config_SelectSDCard()`/`Config_SelectRFID()`/`Config_DeselectAll()`
- **Boot Pins**: GPIO0 HIGH, GPIO15 LOW at boot
- **Memory**: ESP8266 limited RAM - use `yield()` in loops, avoid large String concatenations
- **EEPROM**: Call `EEPROM.commit()` after writes
- **HTTP Redirects**: Use 302 status, not 200 with meta refresh

## Key Files
- **Syntry.ino**: Main loop, mode dispatcher (`catch_Rfid()`)
- **Config.h**: Pin defs, EEPROM, SPI arbitration
- **Hotspot.h**: Web server (650 lines, 25+ routes)
- **Helper.h**: HTML generation (533 lines, 14 page builders)
- **Rfid.h**: Card reading with self-recovery
- **SDCard.h**: File I/O with SPI protection
