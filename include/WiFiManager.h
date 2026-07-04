#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

class WiFiManager
{
public:

    bool begin();

    void handleClient();

private:

    const char* ssid = "AGRI-X";
    const char* password = "12345678";

    WebServer server{80};

    void setupRoutes();

    void handleForward();
    void handleBackward();
    void handleLeft();
    void handleRight();
    void handleStop();

    void serveIndex();
    void serveStyle();
    void serveScript();
};