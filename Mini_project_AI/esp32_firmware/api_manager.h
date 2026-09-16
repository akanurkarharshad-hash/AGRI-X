// API Manager for ESP32
// Handles HTTP POST to backend API
// Sends sensor readings to /api/plots/<plot_id>/readings/sensor endpoint

#ifndef AGRIX_API_MANAGER_H
#define AGRIX_API_MANAGER_H

#include "Arduino.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "config.h"

class APIManager {
private:
    HTTPClient http_client;
    bool last_upload_success;
    unsigned long last_upload_ms;
    int upload_error_count;
    String last_api_error;
    
public:
    APIManager() 
        : last_upload_success(false),
          last_upload_ms(0),
          upload_error_count(0),
          last_api_error("") {}
    
    // Upload sensor reading to backend
    bool uploadReading(const String& json_payload) {
        if (!WiFi.isConnected()) {
            last_api_error = "WiFi not connected";
            upload_error_count++;
            DEBUG_PRINTLN("[AGRI-X][API] Not connected to WiFi");
            return false;
        }
        
        DEBUG_PRINTLN("[AGRI-X][API] Uploading reading...");
        DEBUG_PRINTF("[AGRI-X][API] Payload: %s\n", json_payload.c_str());
        
        // Build full URL - construct endpoint dynamically from PLOT_ID
        String endpoint = "/api/plots/" + String(PLOT_ID) + "/readings/sensor";
        String url = "http://" + String(BACKEND_HOST) + ":" + String(BACKEND_PORT) + endpoint;
        DEBUG_PRINTF("[AGRI-X][API] URL: %s\n", url.c_str());
        
        http_client.begin(url);
        http_client.setTimeout(BACKEND_CONNECT_TIMEOUT_MS);
        http_client.addHeader("Content-Type", "application/json");
        
        // Send POST request
        int http_code = http_client.POST(json_payload);
        
        DEBUG_PRINTF("[AGRI-X][API] HTTP Code: %d\n", http_code);
        
        if (http_code == HTTP_CODE_CREATED) {  // 201
            String response = http_client.getString();
            DEBUG_PRINTF("[AGRI-X][API] Response: %s\n", response.c_str());
            
            http_client.end();
            last_upload_success = true;
            last_upload_ms = millis();
            upload_error_count = 0;
            last_api_error = "";
            
            DEBUG_PRINTLN("[AGRI-X][API] Upload successful");
            return true;
        } else if (http_code > 0) {
            String response = http_client.getString();
            DEBUG_PRINTF("[AGRI-X][API] Error response: %s\n", response.c_str());
            last_api_error = "HTTP " + String(http_code);
        } else {
            last_api_error = "Connection failed";
            DEBUG_PRINTF("[AGRI-X][API] Error: %s\n", http_client.errorToString(http_code).c_str());
        }
        
        http_client.end();
        
        upload_error_count++;
        last_upload_success = false;
        
        DEBUG_PRINTLN("[AGRI-X][API] Upload failed");
        return false;
    }
    
    bool wasLastUploadSuccessful() {
        return last_upload_success;
    }
    
    String getStatus() {
        if (last_upload_success) {
            unsigned long seconds_ago = (millis() - last_upload_ms) / 1000;
            return String(seconds_ago) + "s ago";
        }
        return "Never";
    }
    
    int getErrorCount() {
        return upload_error_count;
    }
    
    String getLastError() {
        return last_api_error;
    }
    
    bool isHealthy() {
        return upload_error_count < WIFI_TIMEOUT_THRESHOLD;
    }
};

#endif // AGRIX_API_MANAGER_H
