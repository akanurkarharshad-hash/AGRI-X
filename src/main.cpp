#include <Arduino.h>

#include "Version.h"
#include "Pins.h"
#include "ProbeMotor.h"
#include "Rover.h"
#include "WiFiManager.h"
#include "DHTSensor.h"
#include "GPSManager.h"
#include "NPKSensor.h"

ProbeMotor probeMotor;
Rover rover;
WiFiManager wifi;
DHTSensor dhtSensor;
GPSManager gps;
NPKSensor npk;

void initializeGPIO()
{
    pinMode(Pins::STATUS_LED, OUTPUT);
    digitalWrite(Pins::STATUS_LED, LOW);
}

void setup()
{
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

    probeMotor.begin();

    dhtSensor.begin();

    gps.begin();

    npk.begin();

    wifi.begin();

    Serial.println("System Ready");
}

void loop()
{
    dhtSensor.update();

    gps.update();

    npk.update();

    static unsigned long lastPrint = 0;

    if (millis() - lastPrint >= 1000)
    {
        lastPrint = millis();

        Serial.println();

        Serial.println("============= SENSOR DATA =============");

        Serial.print("Temperature : ");
        Serial.print(dhtSensor.getTemperature());
        Serial.println(" C");

        Serial.print("Humidity    : ");
        Serial.print(dhtSensor.getHumidity());
        Serial.println(" %");

        Serial.println();

        if (gps.hasFix())
        {
            Serial.println("GPS FIX : YES");

            Serial.print("Latitude    : ");
            Serial.println(gps.getLatitude(), 6);

            Serial.print("Longitude   : ");
            Serial.println(gps.getLongitude(), 6);

            Serial.print("Altitude    : ");
            Serial.print(gps.getAltitude());
            Serial.println(" m");

            Serial.print("Speed       : ");
            Serial.print(gps.getSpeed());
            Serial.println(" km/h");

            Serial.print("Satellites  : ");
            Serial.println(gps.getSatellites());
        }
        else
        {
            Serial.println("GPS FIX : NO");
            Serial.println("Waiting for satellites...");
        }

        Serial.println();

        Serial.println("----------- SOIL SENSOR -----------");

        Serial.print("Moisture : ");
        Serial.print(npk.getMoisture());
        Serial.println(" %");

        Serial.print("Soil Temp : ");
        Serial.print(npk.getTemperature());
        Serial.println(" C");

        Serial.print("EC : ");
        Serial.println(npk.getEC());

        Serial.print("pH : ");
        Serial.println(npk.getPH());

        Serial.print("Nitrogen : ");
        Serial.println(npk.getNitrogen());

        Serial.print("Phosphorus : ");
        Serial.println(npk.getPhosphorus());

        Serial.print("Potassium : ");
        Serial.println(npk.getPotassium());

        Serial.println("-----------------------------------");

        Serial.println("=======================================");
    }
}