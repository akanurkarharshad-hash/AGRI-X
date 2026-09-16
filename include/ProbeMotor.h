#pragma once

#include <Arduino.h>

class ProbeMotor
{
public:

    bool begin();

    void insert();

    void retract();

    void stop();

private:

    void run(bool direction, uint8_t speed = 255);
};