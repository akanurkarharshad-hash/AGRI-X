#include "WiFiManager.h"

WiFiManager wifi;
#include <Arduino.h>
#include "Version.h"
#include "Pins.h"
#include "Rover.h"

Rover rover;

void initializeGPIO()
{
    pinMode(Pins::STATUS_LED, OUTPUT);
    digitalWrite(Pins::STATUS_LED, LOW);
}

void setup()
{
    wifi.begin();
    Serial.begin(115200);
    delay(1000);

    initializeGPIO();

    Serial.println();
    Serial.println("========================================");
    Serial.println("AGRI-X");
    Serial.println("AI Smart Agricultural Rover");
    Serial.println("----------------------------------------");
    Serial.print("Version : ");
   Serial.println(PROJECT_VERSION);
    Serial.println("========================================");

    rover.begin();
    wifi.begin();

    Serial.println("System Ready");
}

void loop()
{
    wifi.handleClient();
}