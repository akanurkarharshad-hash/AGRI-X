// AGRI-X ESP32 Smart Soil Monitoring Firmware
// Configuration Template - Copy this to config.h and fill in your values
// 
// REQUIRED CHANGES: You MUST modify the values marked "*** CHANGE THIS ***"
// before uploading the firmware to your ESP32.

#ifndef AGRIX_CONFIG_H
#define AGRIX_CONFIG_H

// ============================================================================
// FIRMWARE IDENTITY - Set unique values for each device
// ============================================================================

// *** CHANGE THIS *** - Unique identifier for this device (e.g., AGRIX-ESP32-01, AGRIX-ESP32-02)
#define DEVICE_ID "AGRIX-ESP32-01"

// *** CHANGE THIS *** - Which agricultural plot this device is monitoring (e.g., P01, P02, P03)
#define PLOT_ID "P01"

#define FIRMWARE_VERSION "1.0.0"


// ============================================================================
// HARDWARE CONFIGURATION
// These values should match your actual wiring. Only change if using different GPIO pins.
// ============================================================================

// ESP32 UART Configuration (Hardware UART2) - Modbus RTU sensor communication
#define MODBUS_UART_NUM UART_NUM_2
#define MODBUS_BAUD_RATE 4800           // Must match ZTS-3002 sensor (4800 baud)
#define MODBUS_DATA_BITS UART_DATA_8_BITS
#define MODBUS_STOP_BITS UART_STOP_BITS_1
#define MODBUS_PARITY UART_PARITY_DISABLE

// RS485 TTL Converter GPIO Pins (verify your actual wiring matches these)
#define RS485_TX_PIN 17                 // ESP32 GPIO17 → RS485 DI (Data In)
#define RS485_RX_PIN 16                 // ESP32 GPIO16 → RS485 RO (Receive Out)
#define RS485_DE_PIN 4                  // ESP32 GPIO4 → RS485 DE (Driver Enable / Transmit)
#define RS485_RE_PIN 5                  // ESP32 GPIO5 → RS485 RE (Receive Enable, active LOW)

// SSD1306 OLED Display (I2C) - Optional local status display
#define OLED_SDA_PIN 21                 // ESP32 GPIO21 → OLED SDA (I2C Data)
#define OLED_SCL_PIN 22                 // ESP32 GPIO22 → OLED SCL (I2C Clock)
#define OLED_I2C_ADDR 0x3C              // I2C address: 0x3C (or 0x3D if not working)
#define OLED_WIDTH 128                  // SSD1306 display width
#define OLED_HEIGHT 64                  // SSD1306 display height


// ============================================================================
// SENSOR CONFIGURATION
// ZTS-3002-TR-THNPKPH-N01 Soil Parameter Sensor (Modbus RTU)
// These values should not change unless using a different sensor model
// ============================================================================

#define SENSOR_SLAVE_ADDRESS 1          // Modbus slave address (default from factory: 1)
#define SENSOR_REQUEST_TIMEOUT_MS 2000  // Wait 2000ms for sensor response
#define SENSOR_RETRY_COUNT 3            // Retry up to 3 times per sensor poll

// Modbus Function Code
#define MODBUS_FC_READ_HOLDING 0x03     // Read Holding Registers
#define MODBUS_CRC_LEN 2

// Documented registers (from ZTS-3002 manual)
#define REG_MOISTURE 0x0000             // Moisture: 0x0292 (658) → 65.8%
#define REG_TEMPERATURE 0x0001          // Temperature: 0xFF9B (-101) → -10.1°C
#define REG_EC 0x0002                   // EC: 0x03E8 (1000) → 1000 µS/cm
#define REG_PH 0x0003                   // pH: 0x0038 (56) → 5.6

#define NUM_REGISTERS_TO_READ 4         // Read all 4 parameters in one request


// ============================================================================
// CONNECTIVITY CONFIGURATION - MOST IMPORTANT: Change these for your network
// ============================================================================

// WiFi Network Credentials
// *** CHANGE THIS *** - Your WiFi network name (SSID)
#define WIFI_SSID "YourNetworkName"

// *** CHANGE THIS *** - Your WiFi password
#define WIFI_PASSWORD "YourPassword"

// WiFi timing
#define WIFI_CONNECT_TIMEOUT_MS 10000   // Wait 10 seconds for initial WiFi connection
#define WIFI_RECONNECT_INTERVAL_MS 30000 // Try to reconnect every 30 seconds if disconnected

// Backend API Server
// *** CHANGE THIS *** - IP address of your Flask backend server
#define BACKEND_HOST "192.168.1.100"    // Example: 192.168.1.100 or your_server.com

// *** CHANGE THIS IF NEEDED *** - Flask port (default is 5000)
#define BACKEND_PORT 5000

// Backend API timing
#define BACKEND_CONNECT_TIMEOUT_MS 5000 // Wait 5 seconds to connect to API
#define BACKEND_READ_TIMEOUT_MS 5000    // Wait 5 seconds for API response

// NOTE: API endpoint is constructed dynamically from PLOT_ID
// Example: With PLOT_ID="P01" → /api/plots/P01/readings/sensor
// This allows sending data to different plots without code changes


// ============================================================================
// POLLING INTERVALS - How often to read sensor and upload data
// ============================================================================

#define SENSOR_POLL_INTERVAL_MS 10000   // Read ZTS-3002 sensor every 10 seconds
#define BACKEND_UPLOAD_INTERVAL_MS 30000 // Upload to backend every 30 seconds
#define OLED_REFRESH_INTERVAL_MS 1000   // Update OLED display every 1 second


// ============================================================================
// ERROR HANDLING & RESILIENCE
// ============================================================================

#define ERROR_HISTORY_SIZE 10           // Track last 10 errors
#define SENSOR_TIMEOUT_THRESHOLD 5      // Mark sensor as failed after 5 consecutive errors
#define WIFI_TIMEOUT_THRESHOLD 10       // Mark WiFi as failed after 10 consecutive errors


// ============================================================================
// DISPLAY CONFIGURATION
// OLED will cycle through different information screens automatically
// ============================================================================

#define SCREEN_BOOT 0                   // Boot/initialization screen
#define SCREEN_STATUS 1                 // Sensor/WiFi/API status indicators
#define SCREEN_SENSOR_DATA 2            // Current sensor readings (moisture, temp, EC, pH)
#define SCREEN_CONNECTIVITY 3           // WiFi and API upload status
#define SCREEN_ERROR 4                  // Error messages display
#define SCREEN_LAST SCREEN_ERROR
#define SCREEN_CYCLE_MS 5000            // Display each screen for 5 seconds


// ============================================================================
// DEBUGGING - Serial monitor output
// ============================================================================

#define SERIAL_BAUD_RATE 115200         // Serial monitor baud rate for debug output
#define DEBUG_ENABLED 1                 // Set to 0 to disable debug output

#if DEBUG_ENABLED
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(fmt, ...)
#endif


// ============================================================================
// SENSOR VALUE SCALING
// Documented from ZTS-3002 manual
// These conversions convert raw 16-bit register values to sensor measurements
// ============================================================================

#define MOISTURE_SCALE 10.0             // Moisture: raw / 10 = % (658 → 65.8%)
#define TEMPERATURE_SCALE 10.0          // Temperature: raw / 10 = °C (-101 → -10.1°C)
#define EC_SCALE 1.0                    // EC: raw * 1 = µS/cm (1000 → 1000 µS/cm)
#define PH_SCALE 10.0                   // pH: raw / 10 = pH units (56 → 5.6)


#endif // AGRIX_CONFIG_H


// ============================================================================
// SETUP CHECKLIST - Before uploading firmware
// ============================================================================
// 
// [ ] Edit DEVICE_ID: Unique name for this device (e.g., AGRIX-ESP32-01)
// [ ] Edit PLOT_ID: Which plot this device monitors (e.g., P01, P02, P03)
// [ ] Edit WIFI_SSID: Your WiFi network name
// [ ] Edit WIFI_PASSWORD: Your WiFi password
// [ ] Edit BACKEND_HOST: IP address of your Flask backend server
// [ ] Verify GPIO pins match your wiring:
//     - RS485_TX_PIN = 17 (to RS485 DI)
//     - RS485_RX_PIN = 16 (from RS485 RO)
//     - RS485_DE_PIN = 4 (to RS485 DE)
//     - RS485_RE_PIN = 5 (to RS485 RE)
//     - OLED_SDA_PIN = 21 (if using OLED display)
//     - OLED_SCL_PIN = 22 (if using OLED display)
// [ ] Verify sensor address is 1 (SENSOR_SLAVE_ADDRESS)
// [ ] Check baud rate is 4800 (MODBUS_BAUD_RATE)
// [ ] Verify WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
// [ ] Test connectivity: ping your backend server from the same network
// [ ] Install required Arduino libraries: Adafruit_SSD1306, Adafruit_GFX
// [ ] Upload firmware to ESP32 using Arduino IDE
// [ ] Monitor Serial output at 115200 baud to verify operation
//
// ============================================================================
// EXPECTED SERIAL OUTPUT (after successful setup)
// ============================================================================
//
// ========================================
// AGRI-X ESP32 Smart Soil Monitor
// Firmware v1.0.0
// Device ID: AGRIX-ESP32-01
// Plot ID: P01
// ========================================
// 
// [BOOT] Initializing hardware...
// [AGRI-X] Modbus UART initialized
// [AGRI-X] Display initializing...
// [AGRI-X] Display ready
// [AGRI-X] WiFi starting...
// [AGRI-X] Sensor polling...
// [AGRI-X] Sensor OK
// [AGRI-X]   Moisture: 65.8%
// [AGRI-X]   Temperature: -10.1°C
// [AGRI-X]   EC: 1000µS/cm
// [AGRI-X]   pH: 5.6
// [AGRI-X][API] Uploading reading...
// [AGRI-X][API] HTTP Code: 201
//
// ============================================================================
