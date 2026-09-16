#pragma once

#include <Arduino.h>
#include <DHT.h>

// ===============================
// DHT11 Configuration
// ===============================

#define DHT_PIN 4
#define DHT_TYPE DHT11

class DHTSensor
{
public:

    DHTSensor();

    bool begin();

    void update();

    float getTemperature() const;

    float getHumidity() const;

private:

    DHT dht;

    float temperature;

    float humidity;

    unsigned long lastRead;
};