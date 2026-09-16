# AGRI-X ESP32 Smart Soil Monitoring - PlatformIO Edition

A complete PlatformIO-based firmware for the AGRI-X smart soil monitoring system running on ESP32 DevKit V1.

## Requirements

- **VS Code** with PlatformIO IDE extension
- **ESP32 DevKit V1** connected via USB
- **Python 3.6+** (required by PlatformIO)
- **USB drivers** for ESP32 (CH340 or CP2102)

## Quick Start

### 1. Install PlatformIO IDE Extension
- Open VS Code → Extensions (Ctrl+Shift+X)
- Search "PlatformIO IDE" → Install
- Reload VS Code

### 2. Open Project
- File → Open Folder
- Select `esp32_firmware/` folder

### 3. Configure
- Edit `include/config.h`
- Set: DEVICE_ID, PLOT_ID, WIFI_SSID, WIFI_PASSWORD, BACKEND_HOST, BACKEND_PORT

### 4. Connect ESP32 via USB

### 5. Build & Upload
```bash
pio run              # Build
pio run -t upload    # Upload to ESP32
pio device monitor   # View serial output (115200 baud)
```

Or use VS Code PlatformIO toolbar buttons.

## Hardware Setup

### Components Required
1. **ESP32 DevKit V1** - Main microcontroller
2. **ZTS-3002-TR-THNPKPH-N01** - Modbus RTU soil sensor
3. **TTL-to-RS485 Converter (DI/DE/RE/RO)** - RS485 interface
4. **SSD1306 OLED 128x64** - Local status display (optional)
5. **Jumper wires and connectors**

### Wiring Diagram

```
SENSOR → RS485 CONVERTER → ESP32 + OLED

ZTS-3002 (RS485):
├─ A → RS485 A (pin 1)
├─ B → RS485 B (pin 2)
└─ GND → GND

TTL-to-RS485 Converter:
├─ VCC → 5V (or 3.3V, check converter specs)
├─ GND → GND
├─ DI (Data In) → ESP32 GPIO17 (UART2 TX)
├─ RO (Receive Out) → ESP32 GPIO16 (UART2 RX)
├─ DE (Driver Enable) → ESP32 GPIO4
├─ RE (Receive Enable) → ESP32 GPIO5
└─ A/B → To sensor

SSD1306 OLED (I2C):
├─ VCC → 3.3V
├─ GND → GND
├─ SDA → ESP32 GPIO21
└─ SCL → ESP32 GPIO22

ESP32 Pins Used:
GPIO4   - RS485 DE (Direction Enable)
GPIO5   - RS485 RE (Receive Enable)
GPIO16  - UART2 RX (Modbus input)
GPIO17  - UART2 TX (Modbus output)
GPIO21  - I2C SDA (OLED)
GPIO22  - I2C SCL (OLED)
```

### Pin Configuration

Edit `config.h` if your wiring differs:

```cpp
#define RS485_TX_PIN 17         // ESP32 UART2 TX → RS485 DI
#define RS485_RX_PIN 16         // ESP32 UART2 RX → RS485 RO
#define RS485_DE_PIN 4          // Direction Enable
#define RS485_RE_PIN 5          // Receive Enable (active LOW)

#define OLED_SDA_PIN 21         // I2C SDA
#define OLED_SCL_PIN 22         // I2C SCL
```

## Software Setup

### Required Libraries (Arduino IDE)

1. **Arduino Core for ESP32** (built-in)
2. **Adafruit_SSD1306** (for OLED display)
   - Sketch → Include Library → Manage Libraries
   - Search: "Adafruit SSD1306"
   - Install by Adafruit Industries

3. **Adafruit_GFX** (graphics library, auto-installed with SSD1306)

4. **WiFi** (built-in ESP32)

5. **HTTPClient** (built-in ESP32)

### Installation Steps

1. **Clone/Copy Firmware Files**
   ```
   esp32_firmware/
   ├── config.h
   ├── modbus_rtу.h
   ├── rs485_manager.h
   ├── sensor_manager.h
   ├── wifi_manager.h
   ├── api_manager.h
   ├── display_manager.h
   └── agri_x_esp32.ino
   ```

2. **Create Arduino Sketch**
   - In Arduino IDE: File → New
   - Copy-paste contents of `agri_x_esp32.ino`
   - Save as `agri_x_esp32.ino`
   - Create folder `agri_x_esp32/`
   - Copy all `.h` files into that folder

3. **Configure Settings** (MUST CHANGE)
   - Edit `config.h`:
     - `WIFI_SSID` - Your WiFi network name
     - `WIFI_PASSWORD` - Your WiFi password
     - `BACKEND_HOST` - IP address of Flask backend (192.168.1.100)
     - `BACKEND_PORT` - Flask port (5000)
     - `PLOT_ID` - Which plot to upload to (P01, P02, etc.)
     - `DEVICE_ID` - Unique ID for this device

4. **Board Setup** (Arduino IDE)
   - Tools → Board → Select "ESP32 Dev Module"
   - Tools → Upload Speed → 115200
   - Tools → Flash Frequency → 80MHz
   - Tools → Flash Mode → QIO
   - Tools → Partition Scheme → Default 4MB with spiffs
   - Tools → Port → Select COM port of ESP32

5. **Compile and Upload**
   - Sketch → Verify (to check for errors)
   - Sketch → Upload (to upload to device)
   - After upload, open Tools → Serial Monitor
   - Set baud rate to 115200
   - Should see debug output like:
     ```
     ========================================
     AGRI-X ESP32 Smart Soil Monitor
     Firmware v1.0.0
     Device ID: AGRIX-ESP32-01
     Plot ID: P01
     ========================================
     
     [BOOT] Initializing hardware...
     [AGRI-X] Modbus UART initialized
     ...
     ```

## Operation

### Boot Sequence
1. Serial initialization (115200 baud)
2. RS485 hardware init (4800 baud)
3. OLED display init (I2C)
4. WiFi connection attempt
5. Main loop starts

### Normal Operation
- **Every 10 seconds**: Poll ZTS-3002 sensor for readings
- **Every 30 seconds**: Upload readings to backend (if WiFi connected)
- **Every 1 second**: Update OLED display (cycles through 5 screens)

### Display Screens
1. **Status Screen**: Sensor/WiFi/API status indicators
2. **Sensor Data**: Current moisture, temperature, EC, pH values
3. **Connectivity**: WiFi and API upload status
4. **Error Screen**: Shows last error message (if any)

### Serial Debug Output
- All operations logged to Serial at 115200 baud
- Shows sensor polling, Modbus communication, WiFi connection, API uploads
- Enable/disable with `DEBUG_ENABLED` in config.h

## Troubleshooting

### Sensor Not Responding
- Check RS485 wiring (A/B connections)
- Verify UART2 pins (GPIO16/17) are not used by other peripherals
- Check DE/RE pin control (should see "Sent N bytes" in serial log)
- Try increasing `SENSOR_REQUEST_TIMEOUT_MS` if sensor is slow

### WiFi Not Connecting
- Verify SSID and password in config.h
- Check WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Monitor Serial output for connection attempts
- May need to adjust `WIFI_RECONNECT_INTERVAL_MS` for your network

### OLED Display Not Working
- Check I2C address (default 0x3C, try 0x3D if not working)
- Verify SDA/SCL wiring and pullup resistors
- Firmware continues without display (not critical)
- Monitor Serial output for "Display initialization failed"

### Backend Not Receiving Data
- Verify backend Flask app is running
- Check `BACKEND_HOST` and `BACKEND_PORT` in config.h
- Monitor Serial output for "HTTP Code" in API upload logs
- Check Flask app error logs for malformed JSON

## Configuration Reference

### Modbus Configuration
```cpp
#define MODBUS_BAUD_RATE 4800           // Must match sensor: 4800
#define MODBUS_DATA_BITS UART_DATA_8_BITS
#define MODBUS_STOP_BITS UART_STOP_BITS_1
#define MODBUS_PARITY UART_PARITY_DISABLE
#define SENSOR_SLAVE_ADDRESS 1          // Default for ZTS-3002
```

### Timing Configuration
```cpp
#define SENSOR_POLL_INTERVAL_MS 10000       // Poll every 10 seconds
#define BACKEND_UPLOAD_INTERVAL_MS 30000    // Upload every 30 seconds
#define OLED_REFRESH_INTERVAL_MS 1000       // Update display every 1 second
#define SENSOR_REQUEST_TIMEOUT_MS 2000      // Wait 2 seconds for sensor response
```

### Sensor Scaling
```cpp
Moisture:    Raw / 10 = % (e.g., 658 → 65.8%)
Temperature: Raw / 10 = °C (e.g., -101 → -10.1°C) [SIGNED]
EC:          Raw = µS/cm (e.g., 1000 → 1000 µS/cm)
pH:          Raw / 10 = pH (e.g., 56 → 5.6)
```

## File Structure

### config.h
- Hardware pins (UART, RS485, I2C)
- WiFi credentials (MUST CHANGE)
- Backend API configuration
- Sensor settings and timeouts

### modbus_rtу.h
- CRC16 calculation
- Modbus RTU request builder
- Response parser
- Register-to-measurement conversion

### rs485_manager.h
- DE/RE pin control
- Half-duplex direction switching
- Timing for stable transitions

### sensor_manager.h
- Modbus polling orchestration
- Timeout and retry logic
- Error tracking
- JSON payload generation

### wifi_manager.h
- Non-blocking WiFi connection
- Auto-reconnect logic
- Status reporting

### api_manager.h
- HTTP POST to backend
- Error handling and retries
- Upload status tracking

### display_manager.h
- SSD1306 OLED control
- Multi-screen cycling
- Status display formatting

### agri_x_esp32.ino
- Main loop orchestration
- Timing management
- Integration of all modules

## Testing

### Unit Tests (Manual)
1. **Modbus CRC**: Verify against manual calculation
   - Request: `01 03 00 00 00 04 44 09`
   - Expected CRC: `0x0944` (last 2 bytes)

2. **Sensor Communication**:
   - Check Serial output: "Received X bytes from sensor"
   - Check "CRC verification passed"

3. **WiFi Connection**:
   - Serial: "WiFi connected"
   - Check displayed IP address

4. **API Upload**:
   - Serial: "HTTP Code: 201"
   - Check Flask app receives data: `plots.json`

5. **OLED Display**:
   - See text updating every 5 seconds
   - All 5 screens visible after 25 seconds

### Integration Tests
- [x] Hardware initializes without errors
- [x] Sensor reads all 4 parameters (moisture, temp, EC, pH)
- [x] Readings scale correctly per documentation
- [x] WiFi connects and stays connected
- [x] API uploads JSON with correct format
- [x] Backend receives and stores readings
- [x] Dashboard displays new sensor readings
- [x] Manual readings still work (not affected by sensor)
- [x] OLED displays status and error messages
- [x] System recovers from transient sensor errors
- [x] System recovers from WiFi disconnection

## Known Limitations

- Temperature readings in 0.1°C increments
- Moisture readings in 0.1% increments  
- EC readings are integers (no decimal places)
- pH readings in 0.1 units
- Sensor polling every 10 seconds (adjustable)
- No on-device data logging
- No MQTT fallback (WiFi only)
- NPK data NOT available from this sensor (marked as "not_validated")

## Next Steps

1. **Verify Hardware Wiring**: Test with continuity meter before powering on
2. **Upload Firmware**: Follow installation steps above
3. **Test Connectivity**: Monitor Serial output during boot
4. **Verify Sensor Reading**: Check Serial for "Sensor OK" messages
5. **Check Backend**: Monitor Flask app logs for incoming data
6. **Test Dashboard**: Refresh browser to see new sensor readings

## Support

- Check Serial Monitor (115200 baud) for detailed debug output
- All errors prefixed with `[AGRI-X][ERROR]` or `[AGRI-X][WARNING]`
- Enable `DEBUG_ENABLED = 1` in config.h for verbose logging
- Check wiring diagram matches your setup exactly
