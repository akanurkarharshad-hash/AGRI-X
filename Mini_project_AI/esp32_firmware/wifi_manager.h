// WiFi Manager for ESP32
// Handles non-blocking WiFi connection and reconnection

#ifndef AGRIX_WIFI_MANAGER_H
#define AGRIX_WIFI_MANAGER_H

#include "Arduino.h"
#include "WiFi.h"
#include "config.h"

class WiFiManager {
private:
    bool is_connected;
    unsigned long last_connect_attempt_ms;
    int connection_attempt_count;
    String last_error;
    
public:
    WiFiManager() 
        : is_connected(false), 
          last_connect_attempt_ms(0),
          connection_attempt_count(0),
          last_error("") {}
    
    void begin() {
        DEBUG_PRINTLN("[AGRI-X] WiFi starting...");
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    
    // Non-blocking WiFi connection attempt
    // Call this regularly in your main loop
    bool update() {
        if (is_connected) {
            // Check if still connected
            if (WiFi.status() == WL_CONNECTED) {
                return true;
            } else {
                is_connected = false;
                DEBUG_PRINTLN("[AGRI-X][WARNING] WiFi disconnected");
            }
        }
        
        // Attempt to reconnect at intervals
        if (millis() - last_connect_attempt_ms < WIFI_RECONNECT_INTERVAL_MS) {
            return false;
        }
        
        last_connect_attempt_ms = millis();
        connection_attempt_count++;
        
        DEBUG_PRINTF("[AGRI-X] WiFi connect attempt %d...\n", connection_attempt_count);
        
        // Set timeout and try to connect
        WiFi.setAutoConnect(true);
        WiFi.setAutoReconnect(true);
        
        // Check status
        if (WiFi.status() == WL_CONNECTED) {
            is_connected = true;
            connection_attempt_count = 0;
            last_error = "";
            DEBUG_PRINTF("[AGRI-X] WiFi connected\n");
            DEBUG_PRINTF("[AGRI-X] SSID: %s\n", WiFi.SSID().c_str());
            DEBUG_PRINTF("[AGRI-X] IP: %s\n", WiFi.localIP().toString().c_str());
            return true;
        }
        
        // Still trying
        wl_status_t status = WiFi.status();
        DEBUG_PRINTF("[AGRI-X] WiFi status: %d\n", status);
        
        return false;
    }
    
    bool isConnected() {
        return is_connected && WiFi.status() == WL_CONNECTED;
    }
    
    String getStatus() {
        if (is_connected) {
            return "Connected";
        }
        return "Offline";
    }
    
    String getIP() {
        if (is_connected) {
            return WiFi.localIP().toString();
        }
        return "0.0.0.0";
    }
};

#endif // AGRIX_WIFI_MANAGER_H
