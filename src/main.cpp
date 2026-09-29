#include <Arduino.h>

#include "Version.h"
#include "Pins.h"
#include "ProbeMotor.h"
#include "Rover.h"
#include "WiFiManager.h"
#include "DHTSensor.h"
#include "GPSManager.h"
#include "NPKSensor.h"
#include "TimingConfig.h"

ProbeMotor probeMotor;
Rover rover;
WiFiManager wifi;
DHTSensor dhtSensor;
GPSManager gps;
NPKSensor npk;

static void sensorTask(void *)
{
    for (;;)
    {
        dhtSensor.update();
        npk.update();
        vTaskDelay(pdMS_TO_TICKS(TimingConfig::SENSOR_TASK_PERIOD_MS));
    }
}

void initializeGPIO()
{
    pinMode(Pins::STATUS_LED, OUTPUT);
    digitalWrite(Pins::STATUS_LED, LOW);
}

void setup()
{
    Serial.begin(115200);
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

    // Sensor transactions and DHT bit timing run separately so they cannot
    // delay command callbacks or the drive watchdog in loop().
    xTaskCreatePinnedToCore(sensorTask, "sensorTask", 4096, nullptr, 1, nullptr, 0);

    wifi.begin();

    Serial.println("System Ready");
}

void loop()
{
    gps.update();
    wifi.service();
}
