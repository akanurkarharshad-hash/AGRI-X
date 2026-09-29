#pragma once

#include <Arduino.h>
#include <stdint.h>

class NPKSensor
{
public:

    bool begin();

    void update();
    bool isValid() const;
    uint32_t getTimestamp() const;

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

    uint32_t lastRead = 0;
    uint32_t requestStarted = 0;
    uint32_t validTimestamp = 0;
    uint8_t response[32] = {};
    uint8_t responseLength = 0;
    bool requestPending = false;
    bool valid = false;

    void startRequest(uint32_t now);
    void finishResponse();
    uint16_t calculateCRC(const uint8_t *data, uint8_t length);
};
