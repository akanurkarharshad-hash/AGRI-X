#include "DHTSensor.h"
#include "TimingConfig.h"

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
    valid = false;
    validTimestamp = 0;

    return true;
}

void DHTSensor::update()
{
    if (millis() - lastRead < TimingConfig::DHT_SAMPLE_INTERVAL_MS)
        return;

    lastRead = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t))
    {
        valid = false;
        return;
    }

    humidity = h;
    validTimestamp = millis();
    valid = true;
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

bool DHTSensor::isValid() const { return valid; }
uint32_t DHTSensor::getTimestamp() const { return validTimestamp; }
