#pragma once

#include <Arduino.h>

class MotorController
{
public:

    bool begin();

    void forward(uint8_t speed);

    void reverse(uint8_t speed);

    void left(uint8_t speed);

    void right(uint8_t speed);

    void stop();

private:

    void leftMotor(bool direction,uint8_t speed);

    void rightMotor(bool direction,uint8_t speed);
};