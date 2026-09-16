#include "DHTSensor.h"

DHTSensor::DHTSensor()
    : dht(DHT_PIN, DHT_TYPE)
{
}

bool DHTSensor::begin()
{
    dht.begin();

    temperature = 0.0f;
    humidity = 0.0f;

    lastRead = 0;

    return true;
}

void DHTSensor::update()
{
    if (millis() - lastRead < 2000)
        return;

    lastRead = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t))
    {
        return;
    }

    humidity = h;
    temperature = t;
}

float DHTSensor::getTemperature() const
{
    return temperature;
}

float DHTSensor::getHumidity() const
{
    return humidity;
}