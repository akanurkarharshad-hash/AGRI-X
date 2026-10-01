#include "WiFiManager.h"
#include "WiFiSetupHTML.h"
#include "UtilsJS.h"
#include "ControlJS.h"
#include "SensorJS.h"
#include "StatusJS.h"
#include "LogJS.h"
#include "DashboardHTML.h"
#include "DashboardCSS.h"
#include "DashboardJS.h"
#include "AIJS.h"
#include "Rover.h"
#include "DHTSensor.h"
#include "GPSManager.h"
#include "NPKSensor.h"
#include "ProbeMotor.h"

#include <ArduinoJson.h>

extern ProbeMotor probeMotor;
extern Rover rover;
extern DHTSensor dhtSensor;
extern GPSManager gps;
extern NPKSensor npk;

int roverSpeed = 180;

bool WiFiManager::begin()
{
    Serial.println();
    Serial.println("=================================");
    Serial.println("Starting AGRI-X WiFi");
    Serial.println("=================================");

    wifiConfig.begin();

    if (wifiConfig.hasCredentials())
    {
        String savedSSID = wifiConfig.getSSID();
        String savedPassword = wifiConfig.getPassword();

        Serial.println("Saved WiFi found");
        Serial.print("Connecting to: ");
        Serial.println(savedSSID);

        WiFi.mode(WIFI_STA);

        WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

        unsigned long start = millis();

        while (WiFi.status() != WL_CONNECTED &&
               millis() - start < 15000)
        {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println();
            Serial.println("WiFi Connected");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
        }
        else
        {
            Serial.println();
            Serial.println("Connection Failed");
            Serial.println("Starting AGRI-X Hotspot");

            WiFi.mode(WIFI_AP);
            WiFi.softAP(ssid, password);

            Serial.print("AP IP: ");
            Serial.println(WiFi.softAPIP());
        }
    }
    else
    {
        Serial.println("No WiFi Saved");
        Serial.println("Starting AGRI-X Hotspot");

        WiFi.mode(WIFI_AP);

        WiFi.softAP(ssid, password);

        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());
    }

    setupRoutes();

    server.begin();

    Serial.println("Async Web Server Started");

    return true;
}
void WiFiManager::handleWiFiSetup(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", wifiSetupHTML);
}

void WiFiManager::setupRoutes()
{
    server.on("/", HTTP_GET,
[this](AsyncWebServerRequest *request)
{
    handleRoot(request);
});
    server.on("/wifi", HTTP_GET,
[this](AsyncWebServerRequest *request)
{
    handleWiFiSetup(request);
});

    server.on("/forward", HTTP_GET,
    [this](AsyncWebServerRequest *request)
    {
        handleForward(request);
    });

    server.on("/backward", HTTP_GET,
    [this](AsyncWebServerRequest *request)
    {
        handleBackward(request);
    });

    server.on("/left", HTTP_GET,
    [this](AsyncWebServerRequest *request)
    {
        handleLeft(request);
    });

    server.on("/right", HTTP_GET,
    [this](AsyncWebServerRequest *request)
    {
        handleRight(request);
    });

    server.on("/stop", HTTP_GET,
    [this](AsyncWebServerRequest *request)
    {
        handleStop(request);
    });

    server.on("/probe_insert", HTTP_GET,
    [this](AsyncWebServerRequest *request)
    {
        handleProbeInsert(request);
    });

    server.on("/probe_retract", HTTP_GET,
    [this](AsyncWebServerRequest *request)
    {
        handleProbeRetract(request);
    });

    server.on("/probe_stop", HTTP_GET,
    [this](AsyncWebServerRequest *request)
    {
        handleProbeStop(request);
    });

    server.on("/sensorData", HTTP_GET,
    [this](AsyncWebServerRequest *request)
    {
        handleSensorData(request);
    });

    server.on("/status", HTTP_GET,
    [this](AsyncWebServerRequest *request)
    {
        handleStatus(request);
    });

    server.on("/speed", HTTP_GET,
    [this](AsyncWebServerRequest *request)
    {
        handleSpeed(request);
    });
    server.on("/savewifi", HTTP_POST,
[this](AsyncWebServerRequest *request)
{
},
NULL,
[this](AsyncWebServerRequest *request,
      uint8_t *data,
      size_t len,
      size_t index,
      size_t total)
{
    handleSaveWiFi(request, data, len);
});
}
void WiFiManager::handleRoot(AsyncWebServerRequest *request)
{
    String html = dashboardHTML;

html.replace("%CSS%", dashboardCSS);

html.replace("%UTILS_JS%", utilsJS);

html.replace("%CONTROL_JS%", controlJS);

html.replace("%SENSOR_JS%", sensorJS);

html.replace("%STATUS_JS%", statusJS);

html.replace("%LOG_JS%", logJS);

html.replace("%AI_JS%", aiJS);

html.replace("%DASHBOARD_JS%", dashboardJS);

    request->send(200, "text/html", html);
}
void WiFiManager::handleSensorData(AsyncWebServerRequest *request)
{
    StaticJsonDocument<512> doc;

    doc["temperature"] = dhtSensor.getTemperature();
    doc["humidity"] = dhtSensor.getHumidity();

    doc["soilTemperature"] = npk.getTemperature();
    doc["moisture"] = npk.getMoisture();
    doc["ph"] = npk.getPH();
    doc["nitrogen"] = npk.getNitrogen();
    doc["phosphorus"] = npk.getPhosphorus();
    doc["potassium"] = npk.getPotassium();
    doc["ec"] = npk.getEC();

    if (gps.hasFix())
    {
        doc["latitude"] = gps.getLatitude();
        doc["longitude"] = gps.getLongitude();
        doc["satellites"] = gps.getSatellites();
        doc["speed"] = gps.getSpeed();
        doc["altitude"] = gps.getAltitude();
    }
    else
    {
        doc["latitude"] = "--";
        doc["longitude"] = "--";
        doc["satellites"] = 0;
        doc["speed"] = 0;
        doc["altitude"] = 0;
    }

    String json;
    serializeJson(doc, json);

    request->send(200, "application/json", json);
}

void WiFiManager::handleStatus(AsyncWebServerRequest *request)
{
    StaticJsonDocument<256> doc;

    doc["wifi"] = "ONLINE";
    doc["gps"] = gps.hasFix() ? "CONNECTED" : "SEARCHING";
    doc["soil"] = "CONNECTED";
    doc["dht"] = "CONNECTED";
    doc["probe"] = "READY";
    doc["rover"] = "READY";

    String json;
    serializeJson(doc, json);

    request->send(200, "application/json", json);
}

void WiFiManager::handleSpeed(AsyncWebServerRequest *request)
{
    if (request->hasParam("value"))
    {
        roverSpeed = request->getParam("value")->value().toInt();
    }

    request->send(200, "text/plain", "OK");
}
void WiFiManager::handleForward(AsyncWebServerRequest *request)
{
    rover.setSpeed(roverSpeed);

    rover.moveForward();

    request->send(200, "text/plain", "FORWARD");
}

void WiFiManager::handleBackward(AsyncWebServerRequest *request)
{
    rover.setSpeed(roverSpeed);

    rover.moveBackward();

    request->send(200, "text/plain", "BACKWARD");
}

void WiFiManager::handleLeft(AsyncWebServerRequest *request)
{
    rover.setSpeed(roverSpeed);

    rover.turnLeft();

    request->send(200, "text/plain", "LEFT");
}

void WiFiManager::handleRight(AsyncWebServerRequest *request)
{
    rover.setSpeed(roverSpeed);

    rover.turnRight();

    request->send(200, "text/plain", "RIGHT");
}

void WiFiManager::handleStop(AsyncWebServerRequest *request)
{
    rover.stop();

    request->send(200, "text/plain", "STOP");
}
void WiFiManager::handleProbeInsert(AsyncWebServerRequest *request)
{
    probeMotor.insert();

    request->send(200, "text/plain", "PROBE INSERT");
}

void WiFiManager::handleProbeRetract(AsyncWebServerRequest *request)
{
    probeMotor.retract();

    request->send(200, "text/plain", "PROBE RETRACT");
}

void WiFiManager::handleProbeStop(AsyncWebServerRequest *request)
{
    probeMotor.stop();

    request->send(200, "text/plain", "PROBE STOP");
}
void WiFiManager::handleSaveWiFi(
    AsyncWebServerRequest *request,
    uint8_t *data,
    size_t len)
{
    JsonDocument doc;

    DeserializationError err =
        deserializeJson(doc, data, len);

    if (err)
    {
        request->send(400, "text/plain", "Invalid JSON");
        return;
    }

    String ssid = doc["ssid"] | "";
    String password = doc["password"] | "";

    wifiConfig.saveCredentials(ssid, password);

    request->send(
        200,
        "text/plain",
        "WiFi Saved. Restarting..."
    );

    delay(1000);

    ESP.restart();
}