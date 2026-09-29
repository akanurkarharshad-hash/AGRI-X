#pragma once

#include "WiFiConfig.h"
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>

class WiFiManager
{
public:

    bool begin();
    void service();

private:

    const char* ssid = "AGRI-X";
    const char* password = "12345678";

    WiFiConfig wifiConfig;
    AsyncWebServer server{80};
    AsyncWebSocket websocket{"/ws"};
    uint32_t lastMovementAt = 0;
    uint32_t lastTelemetryAt = 0;
    uint32_t lastMovementSequence = 0;
    uint32_t lastStopSequence = 0;
    uint32_t controllerClientId = 0;
    bool controllerCommandSeen = false;
    bool watchdogStopped = true;
    bool restartPending = false;
    uint32_t restartAt = 0;
    bool stationConnectPending = false;
    uint32_t stationConnectStarted = 0;

    void setupRoutes();
    void handleWebSocketEvent(AsyncWebSocketClient *client, AwsEventType type,
                              void *arg, uint8_t *data, size_t len);
    void handleMovement(const char *command, uint8_t speed, uint32_t sequence, bool websocketCommand);
    void publishTelemetry(uint32_t now);

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
