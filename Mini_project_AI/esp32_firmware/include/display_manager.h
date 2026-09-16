// OLED Display Manager for SSD1306
// Manages display update and multi-screen cycling

#ifndef AGRIX_DISPLAY_MANAGER_H
#define AGRIX_DISPLAY_MANAGER_H

#include "Arduino.h"
#include "WiFi.h"
#include "config.h"
// Using Adafruit_SSD1306 library
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

class DisplayManager {
private:
    Adafruit_SSD1306* display;
    int current_screen;
    unsigned long last_screen_change_ms;
    bool display_initialized;
    
public:
    DisplayManager()
        : display(nullptr),
          current_screen(SCREEN_BOOT),
          last_screen_change_ms(0),
          display_initialized(false) {}
    
    bool begin() {
        DEBUG_PRINTLN("[AGRI-X] Display initializing...");
        
        // Create display object for SSD1306 (128x64)
        display = new Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
        
        if (!display->begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
            DEBUG_PRINTLN("[AGRI-X][ERROR] SSD1306 initialization failed");
            return false;
        }
        
        display->clearDisplay();
        display->setTextSize(1);
        display->setTextColor(SSD1306_WHITE);
        display->setCursor(0, 0);
        
        display->println("AGRI-X");
        display->println("Smart Soil");
        display->println("Monitoring");
        display->println("");
        display->println("Initializing...");
        display->display();
        
        display_initialized = true;
        last_screen_change_ms = millis();
        
        DEBUG_PRINTLN("[AGRI-X] Display ready");
        return true;
    }
    
    void update(
        const ModbusRTU::SensorReading& sensor,
        bool sensor_ok,
        bool wifi_connected,
        bool api_ok,
        const String& api_status,
        const String& last_error = ""
    ) {
        if (!display_initialized) return;
        
        // Cycle through screens
        if (millis() - last_screen_change_ms > SCREEN_CYCLE_MS) {
            current_screen++;
            if (current_screen > SCREEN_LAST) {
                current_screen = SCREEN_STATUS;
            }
            last_screen_change_ms = millis();
        }
        
        display->clearDisplay();
        display->setTextSize(1);
        display->setTextColor(SSD1306_WHITE);
        display->setCursor(0, 0);
        
        switch (current_screen) {
            case SCREEN_STATUS:
                drawStatusScreen(sensor_ok, wifi_connected, api_ok);
                break;
            
            case SCREEN_SENSOR_DATA:
                drawSensorScreen(sensor, sensor_ok);
                break;
            
            case SCREEN_CONNECTIVITY:
                drawConnectivityScreen(wifi_connected, api_status);
                break;
            
            case SCREEN_NPK:
                drawNPKScreen(sensor, sensor_ok);
                break;
            
            case SCREEN_ERROR:
                if (!last_error.isEmpty()) {
                    drawErrorScreen(last_error);
                } else {
                    drawStatusScreen(sensor_ok, wifi_connected, api_ok);
                }
                break;
            
            default:
                drawStatusScreen(sensor_ok, wifi_connected, api_ok);
        }
        
        display->display();
    }
    
private:
    void drawStatusScreen(bool sensor_ok, bool wifi_ok, bool api_ok) {
        display->setTextSize(2);
        display->println("AGRI-X");
        
        display->setTextSize(1);
        display->println("SMART SOIL");
        display->println("");
        
        display->print("Sensor: ");
        display->println(sensor_ok ? "OK" : "FAIL");
        
        display->print("WiFi:   ");
        display->println(wifi_ok ? "OK" : "OFFLINE");
        
        display->print("API:    ");
        display->println(api_ok ? "OK" : "FAIL");
    }
    
    void drawSensorScreen(const ModbusRTU::SensorReading& sensor, bool valid) {
        display->setTextSize(1);
        display->println("SENSOR DATA");
        display->println("---");
        
        if (!valid) {
            display->println("No valid reading");
            return;
        }
        
        display->print("Moisture: ");
        display->print(sensor.moisture, 1);
        display->println("%");
        
        display->print("Temp: ");
        display->print(sensor.temperature, 1);
        display->println("C");
        
        display->print("EC: ");
        display->print(sensor.ec, 0);
        display->println("us/cm");
        
        display->print("pH: ");
        display->println(sensor.ph, 1);
    }
    
    void drawConnectivityScreen(bool wifi_ok, const String& api_status) {
        display->setTextSize(1);
        display->println("CONNECTIVITY");
        display->println("---");
        
        display->print("WiFi: ");
        display->println(wifi_ok ? "Connected" : "Offline");
        
        if (wifi_ok) {
            display->print("IP: ");
            display->println(WiFi.localIP().toString());
        }
        
        display->println("");
        display->print("Last API: ");
        display->println(api_status);
    }
    
    void drawErrorScreen(const String& error) {
        display->setTextSize(1);
        display->println("SYSTEM ERROR");
        display->println("---");
        
        // Wrap long error messages
        int pos = 0;
        while (pos < error.length()) {
            int chunk = min(16, (int)error.length() - pos);
            display->println(error.substring(pos, pos + chunk));
            pos += chunk;
        }
    }
    
    void drawNPKScreen(const ModbusRTU::SensorReading& sensor, bool valid) {
        display->setTextSize(1);
        display->println("NPK NUTRIENTS");
        display->println("---");
        
        if (!valid) {
            display->println("No valid reading");
            return;
        }
        
        display->print("N: ");
        display->print(sensor.nitrogen);
        display->println(" mg/kg");
        
        display->print("P: ");
        display->print(sensor.phosphorus);
        display->println(" mg/kg");
        
        display->print("K: ");
        display->print(sensor.potassium);
        display->println(" mg/kg");
    }
};

#endif // AGRIX_DISPLAY_MANAGER_H
