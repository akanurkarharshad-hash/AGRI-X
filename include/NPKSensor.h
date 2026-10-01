#pragma once

#include <Arduino.h>

class NPKSensor
{
public:

    bool begin();

    void update();

    float getMoisture() const;
    float getTemperature() const;
    float getPH() const;
    float getEC() const;

    uint16_t getNitrogen() const;
    uint16_t getPhosphorus() const;
    uint16_t getPotassium() const;

private:

    HardwareSerial rs485 = HardwareSerial(1);

    float moisture = 0;
    float temperature = 0;
    float ph = 0;
    float ec = 0;

    uint16_t nitrogen = 0;
    uint16_t phosphorus = 0;
    uint16_t potassium = 0;

    unsigned long lastRead = 0;

    bool readSensor();
    uint16_t calculateCRC(uint8_t *data, uint8_t length);
};