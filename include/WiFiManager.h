#pragma once

#include "WiFiConfig.h"
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

class WiFiManager
{
public:

    bool begin();

private:

    const char* ssid = "AGRI-X";
    const char* password = "12345678";

    WiFiConfig wifiConfig;
    AsyncWebServer server{80};

    void setupRoutes();

    // Dashboard
    void handleRoot(AsyncWebServerRequest *request);

    // WiFi Setup
    void handleWiFiSetup(AsyncWebServerRequest *request);

    void handleSaveWiFi(
        AsyncWebServerRequest *request,
        uint8_t *data,
        size_t len
    );

    // Rover Controls
    void handleForward(AsyncWebServerRequest *request);
    void handleBackward(AsyncWebServerRequest *request);
    void handleLeft(AsyncWebServerRequest *request);
    void handleRight(AsyncWebServerRequest *request);
    void handleStop(AsyncWebServerRequest *request);

    // Probe Controls
    void handleProbeInsert(AsyncWebServerRequest *request);
    void handleProbeRetract(AsyncWebServerRequest *request);
    void handleProbeStop(AsyncWebServerRequest *request);

    // AJAX APIs
    void handleSensorData(AsyncWebServerRequest *request);
    void handleStatus(AsyncWebServerRequest *request);
    void handleSpeed(AsyncWebServerRequest *request);
};