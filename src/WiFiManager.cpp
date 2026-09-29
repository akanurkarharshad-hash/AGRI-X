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
#include "TimingConfig.h"

#include <ArduinoJson.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

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

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println();
            Serial.println("WiFi Connected");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
        }
        else
        {
            stationConnectPending = true;
            stationConnectStarted = millis();
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
    websocket.onEvent([this](AsyncWebSocket *, AsyncWebSocketClient *client,
                             AwsEventType type, void *arg, uint8_t *data, size_t len)
    {
        handleWebSocketEvent(client, type, arg, data, len);
    });
    websocket.handleHandshake([](AsyncWebServerRequest *request)
    {
        if (!request->hasParam("key"))
            return false;
        return request->getParam("key")->value() == TimingConfig::WEBSOCKET_SHARED_KEY;
    });
    server.addHandler(&websocket);

    server.begin();

    Serial.println("Async Web Server Started");

    return true;
}

void WiFiManager::service()
{
    const uint32_t now = millis();
    if (restartPending && now - restartAt >= TimingConfig::WIFI_RESTART_DELAY_MS)
        ESP.restart();
    if (stationConnectPending && WiFi.status() == WL_CONNECTED)
    {
        stationConnectPending = false;
        Serial.print("WiFi Connected; IP Address: ");
        Serial.println(WiFi.localIP());
    }
    else if (stationConnectPending && now - stationConnectStarted >= TimingConfig::WIFI_CONNECT_TIMEOUT_MS)
    {
        stationConnectPending = false;
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssid, password);
        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());
    }
    if (!watchdogStopped && now - lastMovementAt >= TimingConfig::DRIVE_WATCHDOG_MS)
    {
        rover.stop();
        watchdogStopped = true;
    }
    publishTelemetry(now);
    websocket.cleanupClients();
}

static bool sequenceIsNewer(uint32_t candidate, uint32_t previous)
{
    return static_cast<int32_t>(candidate - previous) > 0;
}

void WiFiManager::handleMovement(const char *command, uint8_t speed,
                                 uint32_t sequence, bool websocketCommand)
{
    if (strcmp(command, "S") == 0 || strcmp(command, "STOP") == 0)
    {
        // STOP is applied immediately and invalidates any older queued move.
        lastStopSequence = sequence;
        lastMovementSequence = sequence;
        rover.stop();
        watchdogStopped = true;
        controllerCommandSeen = true;
        return;
    }
    // Deprecated GET movement remains available only during migration when
    // no WebSocket controller owns the rover. STOP remains available always.
    if (!websocketCommand && controllerClientId != 0)
        return;
    if (websocketCommand && (!controllerClientId || !controllerCommandSeen))
    {
        // Controller ownership is assigned at connect; a fresh valid command
        // is required after every connection before movement is allowed.
        if (!controllerClientId)
            return;
    }
    if (websocketCommand && !sequenceIsNewer(sequence, lastMovementSequence))
        return;
    if (websocketCommand && !sequenceIsNewer(sequence, lastStopSequence))
        return;

    rover.setSpeed(speed);
    if (strcmp(command, "F") == 0 || strcmp(command, "FORWARD") == 0)
        rover.moveForward();
    else if (strcmp(command, "B") == 0 || strcmp(command, "BACKWARD") == 0)
        rover.moveBackward();
    else if (strcmp(command, "L") == 0 || strcmp(command, "LEFT") == 0)
        rover.turnLeft();
    else if (strcmp(command, "R") == 0 || strcmp(command, "RIGHT") == 0)
        rover.turnRight();
    else
        return;

    lastMovementSequence = sequence;
    lastMovementAt = millis();
    watchdogStopped = false;
    controllerCommandSeen = true;
}

void WiFiManager::handleWebSocketEvent(AsyncWebSocketClient *client, AwsEventType type,
                                       void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        if (controllerClientId == 0)
        {
            controllerClientId = client->id();
            controllerCommandSeen = false;
            lastMovementSequence = 0;
            lastStopSequence = 0;
            rover.stop();
            watchdogStopped = true;
            client->text("R,controller");
        }
        else
            client->text("R,readonly");
        return;
    }
    if (type == WS_EVT_DISCONNECT)
    {
        if (client->id() == controllerClientId)
        {
            // Losing the controlling socket stops immediately and releases
            // ownership; a reconnect must issue a new movement command.
            rover.stop();
            watchdogStopped = true;
            controllerCommandSeen = false;
            controllerClientId = 0;
        }
        return;
    }
    if (type != WS_EVT_DATA || !client->status())
        return;

    const AwsFrameInfo *info = static_cast<AwsFrameInfo *>(arg);
    if (!info->final || info->index != 0 || info->len != len || info->message_opcode != WS_TEXT || len == 0 || len >= 48)
        return;

    char message[48];
    memcpy(message, data, len);
    message[len] = '\0';
    char typeCode = 0;
    char trailing = 0;
    unsigned long sequence = 0;
    char command[12] = {};
    unsigned int speed = static_cast<unsigned int>(roverSpeed);
    int parsed = sscanf(message, "%c,%lu,%11[^,],%u%c", &typeCode, &sequence, command, &speed, &trailing);
    int stopParsed = sscanf(message, "S,%lu%c", &sequence, &trailing);
    if (typeCode == 'S' && stopParsed == 1)
    {
        // STOP from any authenticated socket takes priority over movement.
        const uint32_t requestedStop = static_cast<uint32_t>(sequence);
        const uint32_t stopBarrier = client->id() == controllerClientId &&
            sequenceIsNewer(requestedStop, lastMovementSequence)
            ? requestedStop : lastMovementSequence;
        handleMovement("S", 0, stopBarrier, true);
        return;
    }
    if (parsed == 4 && typeCode == 'M' && client->id() == controllerClientId && speed <= 255)
        handleMovement(command, static_cast<uint8_t>(speed), static_cast<uint32_t>(sequence), true);
}

void WiFiManager::publishTelemetry(uint32_t now)
{
    if (now - lastTelemetryAt < TimingConfig::TELEMETRY_INTERVAL_MS || websocket.count() == 0)
        return;
    lastTelemetryAt = now;

    // Compact fixed-size JSON keeps telemetry separate from command frames.
    char payload[320];
    const int count = snprintf(payload, sizeof(payload),
        "{\"t\":%.1f,\"h\":%.1f,\"dv\":%u,\"dts\":%lu,\"st\":%.1f,\"m\":%.1f,\"ph\":%.1f,\"ec\":%.0f,\"n\":%u,\"p\":%u,\"k\":%u,\"nv\":%u,\"nts\":%lu,\"lat\":%.6f,\"lon\":%.6f,\"alt\":%.1f,\"gs\":%.1f,\"sat\":%u,\"gps\":%u,\"run\":%u}",
        dhtSensor.getTemperature(), dhtSensor.getHumidity(), dhtSensor.isValid() ? 1U : 0U,
        static_cast<unsigned long>(dhtSensor.getTimestamp()), npk.getTemperature(), npk.getMoisture(),
        npk.getPH(), npk.getEC(), npk.getNitrogen(), npk.getPhosphorus(), npk.getPotassium(),
        npk.isValid() ? 1U : 0U, static_cast<unsigned long>(npk.getTimestamp()),
        gps.getLatitude(), gps.getLongitude(), gps.getAltitude(), gps.getSpeed(),
        static_cast<unsigned int>(gps.getSatellites()), gps.hasFix() ? 1U : 0U,
        watchdogStopped ? 0U : 1U);
    if (count > 0 && count < static_cast<int>(sizeof(payload)))
        websocket.textAll(payload);
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
    html.replace("%WS_KEY%", TimingConfig::WEBSOCKET_SHARED_KEY);
    html.replace("%ESP32_HOST%", "");

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
        roverSpeed = constrain(request->getParam("value")->value().toInt(), 0, 255);
    }

    request->send(200, "text/plain", "OK");
}
void WiFiManager::handleForward(AsyncWebServerRequest *request)
{
    handleMovement("FORWARD", static_cast<uint8_t>(roverSpeed), ++lastMovementSequence, false);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "FORWARD (deprecated; use /ws)");
    response->addHeader("Deprecation", "true"); request->send(response);
}

void WiFiManager::handleBackward(AsyncWebServerRequest *request)
{
    handleMovement("BACKWARD", static_cast<uint8_t>(roverSpeed), ++lastMovementSequence, false);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "BACKWARD (deprecated; use /ws)");
    response->addHeader("Deprecation", "true"); request->send(response);
}

void WiFiManager::handleLeft(AsyncWebServerRequest *request)
{
    handleMovement("LEFT", static_cast<uint8_t>(roverSpeed), ++lastMovementSequence, false);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "LEFT (deprecated; use /ws)");
    response->addHeader("Deprecation", "true"); request->send(response);
}

void WiFiManager::handleRight(AsyncWebServerRequest *request)
{
    handleMovement("RIGHT", static_cast<uint8_t>(roverSpeed), ++lastMovementSequence, false);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "RIGHT (deprecated; use /ws)");
    response->addHeader("Deprecation", "true"); request->send(response);
}

void WiFiManager::handleStop(AsyncWebServerRequest *request)
{
    handleMovement("STOP", 0, lastMovementSequence, false);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "STOP (deprecated; use /ws)");
    response->addHeader("Deprecation", "true"); request->send(response);
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

    restartPending = true;
    restartAt = millis();
}
