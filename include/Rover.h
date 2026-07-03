#pragma once

#include <Arduino.h>
#include "MotorController.h"

class Rover
{
public:

    bool begin();

    void moveForward();
    void moveBackward();

    void turnLeft();
    void turnRight();

    void rotateLeft();
    void rotateRight();

    void stop();

    void setSpeed(uint8_t speed);

    uint8_t getSpeed() const;

private:

    MotorController motors;

    uint8_t currentSpeed = 120;
};