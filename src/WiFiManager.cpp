#include "WiFiManager.h"
#include "Rover.h"

extern Rover rover;

bool WiFiManager::begin()
{
    Serial.println();
    Serial.println("====================================");

    if (!LittleFS.begin(true))
    {
        Serial.println("LittleFS Mount Failed");
        return false;
    }

    WiFi.mode(WIFI_AP);

    WiFi.softAP(ssid, password);

    Serial.println("WiFi Started");
    Serial.print("SSID : ");
    Serial.println(ssid);

    Serial.print("IP : ");
    Serial.println(WiFi.softAPIP());

    setupRoutes();

    server.begin();

    Serial.println("HTTP Server Started");
    Serial.println("====================================");

    return true;
}

void WiFiManager::handleClient()
{
    server.handleClient();
}

void WiFiManager::setupRoutes()
{
    server.on("/", [this]()
    {
        serveIndex();
    });

    server.on("/style.css", [this]()
    {
        serveStyle();
    });

    server.on("/script.js", [this]()
    {
        serveScript();
    });

    server.on("/forward", [this]()
    {
        handleForward();
    });

    server.on("/backward", [this]()
    {
        handleBackward();
    });

    server.on("/left", [this]()
    {
        handleLeft();
    });

    server.on("/right", [this]()
    {
        handleRight();
    });

    server.on("/stop", [this]()
    {
        handleStop();
    });
}

void WiFiManager::serveIndex()
{
    File file = LittleFS.open("/index.html", "r");

    server.streamFile(file, "text/html");

    file.close();
}

void WiFiManager::serveStyle()
{
    File file = LittleFS.open("/style.css", "r");

    server.streamFile(file, "text/css");

    file.close();
}

void WiFiManager::serveScript()
{
    File file = LittleFS.open("/script.js", "r");

    server.streamFile(file, "application/javascript");

    file.close();
}

void WiFiManager::handleForward()
{
    rover.moveForward();
    server.send(200, "text/plain", "Forward");
}

void WiFiManager::handleBackward()
{
    rover.moveBackward();
    server.send(200, "text/plain", "Backward");
}

void WiFiManager::handleLeft()
{
    rover.turnLeft();
    server.send(200, "text/plain", "Left");
}

void WiFiManager::handleRight()
{
    rover.turnRight();
    server.send(200, "text/plain", "Right");
}

void WiFiManager::handleStop()
{
    rover.stop();
    server.send(200, "text/plain", "Stop");
}