// AGRI-X ESP32 Smart Soil Monitoring System
// Main firmware orchestrator
// Integrates: Sensor polling, OLED display, WiFi connectivity, Backend API

// Required Arduino libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "config.h"
#include "modbus_rtу.h"
#include "rs485_manager.h"
#include "sensor_manager.h"
#include "wifi_manager.h"
#include "api_manager.h"
#include "display_manager.h"

// Global object instances
RS485Manager rs485(RS485_DE_PIN, RS485_RE_PIN);
SensorManager sensor_mgr(&rs485, &Serial2);
WiFiManager wifi_mgr;
APIManager api_mgr;
DisplayManager display_mgr;

// Timing variables
unsigned long last_sensor_poll_ms = 0;
unsigned long last_api_upload_ms = 0;
unsigned long last_display_update_ms = 0;

void setup() {
    // Serial for debugging
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);
    
    DEBUG_PRINTLN("\n\n========================================");
    DEBUG_PRINTLN("AGRI-X ESP32 Smart Soil Monitor");
    DEBUG_PRINTF("Firmware v%s\n", FIRMWARE_VERSION);
    DEBUG_PRINTF("Device ID: %s\n", DEVICE_ID);
    DEBUG_PRINTF("Plot ID: %s\n", PLOT_ID);
    DEBUG_PRINTLN("========================================\n");
    
    // Initialize hardware
    DEBUG_PRINTLN("[BOOT] Initializing hardware...");
    rs485.begin();
    sensor_mgr.begin();
    
    // Initialize display
    DEBUG_PRINTLN("[BOOT] Initializing display...");
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    if (!display_mgr.begin()) {
        DEBUG_PRINTLN("[BOOT][ERROR] Display initialization failed - continuing anyway");
    }
    
    // Initialize WiFi
    DEBUG_PRINTLN("[BOOT] Initializing WiFi...");
    wifi_mgr.begin();
    
    DEBUG_PRINTLN("[BOOT] Setup complete\n");
}

void loop() {
    // Non-blocking operations - manage timing with millis()
    unsigned long now_ms = millis();
    
    // WiFi connection management
    wifi_mgr.update();
    
    // Poll sensor at interval
    if (now_ms - last_sensor_poll_ms >= SENSOR_POLL_INTERVAL_MS) {
        last_sensor_poll_ms = now_ms;
        
        // Attempt sensor reading with retries
        bool sensor_ok = false;
        for (int retry = 0; retry < SENSOR_RETRY_COUNT; retry++) {
            if (sensor_mgr.pollSensor()) {
                sensor_ok = true;
                break;
            }
            delay(100);  // Small delay between retries
        }
    }
    
    // Upload to backend at interval
    if (now_ms - last_api_upload_ms >= BACKEND_UPLOAD_INTERVAL_MS) {
        last_api_upload_ms = now_ms;
        
        if (wifi_mgr.isConnected()) {
            // Get current sensor reading and upload
            SensorData data = sensor_mgr.getLastReading();
            if (data.is_valid) {
                String json_payload = sensor_mgr.getReadingJSON();
                api_mgr.uploadReading(json_payload);
            }
        }
    }
    
    // Update display at interval
    if (now_ms - last_display_update_ms >= OLED_REFRESH_INTERVAL_MS) {
        last_display_update_ms = now_ms;
        
        SensorData sensor_data = sensor_mgr.getLastReading();
        bool sensor_ok = sensor_data.is_valid;
        bool wifi_ok = wifi_mgr.isConnected();
        bool api_ok = api_mgr.wasLastUploadSuccessful();
        
        display_mgr.update(
            sensor_data.reading,
            sensor_ok,
            wifi_ok,
            api_ok,
            api_mgr.getStatus(),
            sensor_data.last_error
        );
    }
    
    // Allow system to yield to other tasks
    yield();
}

/*
FIRMWARE ARCHITECTURE NOTES:

1. NON-BLOCKING DESIGN
   - Main loop never calls delay() for long periods
   - All operations use millis() for timing
   - Allows responsive display updates and interrupts

2. SENSOR POLLING (every 10 seconds)
   - Modbus request built with correct CRC16
   - RS485 direction control: TX -> RX after transmission
   - Timeout: 2000ms per poll
   - Retry logic: up to 3 attempts per poll

3. BACKEND API (every 30 seconds)
   - Only uploads when WiFi connected
   - Sends JSON payload to /api/plots/P01/readings/sensor
   - Validates HTTP 201 (Created) response
   - Tracks error counts for health monitoring

4. DISPLAY UPDATES (every 1 second)
   - Cycles through 5 screens automatically
   - Shows sensor data, WiFi/API status, errors
   - 128x64 SSD1306 OLED display

5. WiFi MANAGEMENT
   - Non-blocking auto-reconnect every 30 seconds
   - Sensor continues operating without WiFi
   - API uploads fail gracefully if offline

6. BACKWARD COMPATIBILITY
   - System works without OLED display
   - System works without WiFi (sensor only)
   - System works without valid reading (shows errors)
   - All existing manual readings still work on dashboard

7. NPK HANDLING
   - Sensor does NOT provide NPK data
   - Backend marks npk_validation="not_validated"
   - Analysis pipeline respects this metadata
   - No fabricated nutrient data ever sent

8. ERROR TRACKING
   - Each subsystem tracks error count
   - Thresholds: SENSOR_TIMEOUT_THRESHOLD=5, WIFI_TIMEOUT_THRESHOLD=10
   - Display shows "FAIL" when threshold exceeded
   - Errors logged to Serial for debugging

TESTING CHECKLIST:
✓ Hardware initialization sequence
✓ Modbus CRC16 calculation (verify with manual values)
✓ RS485 half-duplex direction control
✓ Sensor communication with 2000ms timeout
✓ Retry logic for transient failures
✓ OLED display multi-screen cycling
✓ WiFi connection and reconnection
✓ HTTP POST to backend with JSON payload
✓ Error handling and status reporting
✓ Non-blocking main loop (no hangs)

KNOWN LIMITATIONS:
- Temperature sensor readings are in 0.1°C increments
- pH sensor readings are in 0.1 pH increments
- EC readings are integers (no decimals)
- Moisture readings are in 0.1% increments
- No SD card logging (can add later)
- No MQTT fallback (can add later)
- No OTA updates (can add later)
*/
